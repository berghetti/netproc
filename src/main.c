
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <signal.h>  // sigaction
#include <unistd.h>  // STDIN_FILENO
#include <poll.h>    // poll
#include <locale.h>

#include "config.h"
#include "packet.h"
#include "rate.h"
#include "processes.h"
#include "sock.h"
#include "ring.h"
#include "filter.h"
#include "statistics.h"
#include "human_readable.h"
#include "timer.h"
#include "tui.h"
#include "log.h"
#include "usage.h"
#include "m_error.h"
#include "resolver/resolver.h"

// time to refresh in milliseconds
#define T_REFRESH 1000

static void
config_sig_handler ( void );

// handled by function sig_handler
// if != 0 program exit
static volatile sig_atomic_t prog_exit = 0;

int
main ( int argc, char **argv )
{
  setlocale ( LC_CTYPE, "" );  // needle to ncursesw

  struct ring *ring = NULL;
  struct processes *processes = NULL;

  struct config_op *co = parse_options ( argc, argv );

  int sock = socket_init ( co );
  if ( sock == -1 )
    {
      fatal_error ( "Error create socket, is root?" );
      goto EXIT;
    }

  ring = ring_init ( sock );
  if ( !ring )
    {
      fatal_error ( "Error ring_init" );
      goto EXIT;
    }

  // filter BPF
  if ( !filter_set ( sock, co ) )
    {
      fatal_error ( "Error set filter network" );
      goto EXIT;
    }

  if ( co->log && !log_init ( co ) )
    {
      fatal_error ( "Error log_init" );
      goto EXIT;
    }

  processes = processes_init ();
  if ( !processes )
    {
      fatal_error ( "Error process_init" );
      goto EXIT;
    }

  if ( co->translate_host && !resolver_init ( 0, 0 ) )
    {
      fatal_error ( "Error resolver_init" );
      goto EXIT;
    }

  define_sufix ( co );
  if ( !tui_init ( co ) )
    {
      fatal_error ( "Error setup terminal user interface" );
      goto EXIT;
    }

  config_sig_handler ();

  // long m_timer;
  struct timespec m_timer;
  if ( !start_timer ( &m_timer ) )
    {
      fatal_error ( "Error start timer" );
      goto EXIT;
    }

  if ( !processes_get ( processes, co ) )
    {
      fatal_error ( "Error get processes" );
      goto EXIT;
    }

  const nfds_t nfds = 2;
  struct pollfd poll_set[2] = {
          { .fd = STDIN_FILENO, .events = POLLIN, .revents = 0 },
          { .fd = sock, .events = POLLIN | POLLPRI, .revents = 0 } };

  int block_num = 0;
  struct tpacket_block_desc *pbd;
  pbd = ( struct tpacket_block_desc * ) ring->rd[block_num].iov_base;

  // main loop
  while ( !prog_exit )
    {
      struct packet packet;
      bool packtes_reads = false;
      bool need_update_processes = false;

      // read all blocks availables
      while ( pbd->hdr.bh1.block_status & TP_STATUS_USER )
        {
          struct tpacket3_hdr *ppd;
          ppd = ( struct tpacket3_hdr * ) ( ( uint8_t * ) pbd +
                                            pbd->hdr.bh1.offset_to_first_pkt );

          // read all frames of block
          for ( size_t i = 0; i < pbd->hdr.bh1.num_pkts; i++,
                       ppd = ( struct tpacket3_hdr * ) ( ( uint8_t * ) ppd +
                                                         ppd->tp_next_offset ) )
            {
              // se houver dados porem não foi possivel identificar o trafego,
              // não tem estatisticas para ser adicionada aos processos.
              // deve ser trafego de protocolo não suportado
              if ( !parse_packet ( &packet, ppd ) )
                continue;

              // se não for possivel identificar de qual processo o trafego
              // pertence é sinal que existe um novo processo, ou nova conexão
              // de um processo existente, que ainda não foi mapeado, então
              // anotamos que sera necessario atualizar a lista de processos
              // com conexões ativas.
              if ( !statistics_add ( processes, &packet, co ) )
                {
                  need_update_processes = true;
                  continue;
                }

              packtes_reads = true;
            }

          // pass block controller to kernel
          pbd->hdr.bh1.block_status = TP_STATUS_KERNEL;

          // rotate block
          block_num = ( block_num + 1 ) % ring->req.tp_block_nr;
          pbd = ( struct tpacket_block_desc * ) ring->rd[block_num].iov_base;
        }

      if ( !packtes_reads )
        {
          long temp_diff = 0;
          int stop = 0;
          while ( !stop )
            {
              int rp;
              do
                {
                  errno = 0;
                  rp = poll ( poll_set, nfds, T_REFRESH - temp_diff );
                }
              while ( errno == EINTR );

              if ( rp == -1 )
                {
                  ERROR_DEBUG ( "poll: \"%s\"", strerror ( errno ) );
                  goto EXIT;
                }
              else if ( rp > 0 )
                {
                  for ( size_t i = 0; i < nfds; i++ )
                    {
                      if ( !poll_set[i].revents )
                        continue;

                      if ( poll_set[i].fd == STDIN_FILENO &&
                           tui_handle_input ( co ) == P_EXIT )
                        goto EXIT;

                      if ( poll_set[i].fd == sock )
                        stop = 1;
                    }
                }

              if ( ( temp_diff = diff_timer ( &m_timer ) ) >= T_REFRESH )
                {
                  co->running += temp_diff;
                  temp_diff = 0;
                  start_timer ( &m_timer );

                  rate_calc ( processes, co );

                  statistics_prepare ( processes, co );

                  tui_show ( processes, co );

                  if ( co->log &&
                       !log_file ( processes->proc, processes->total ) )
                    {
                      goto EXIT;
                    }

                  if ( need_update_processes &&
                       !processes_get ( processes, co ) )
                    goto EXIT;
                }
            }
        }

    }  // main loop

EXIT:

  socket_free ( sock );
  ring_free ( ring );
  log_free ();
  processes_free ( processes );
  resolver_free ();
  tui_free ();

  return prog_exit;
}

static void
sig_handler ( int sig )
{
  // "The return value of a simple command is its exit status,
  //  or 128+n if the command is terminated by signal n"
  // by man bash
  prog_exit = 128 + sig;
}

static void
config_sig_handler ( void )
{
  struct sigaction sigact = { .sa_handler = sig_handler };
  sigemptyset ( &sigact.sa_mask );

  sigaction ( SIGINT, &sigact, NULL );
  sigaction ( SIGTERM, &sigact, NULL );
}
