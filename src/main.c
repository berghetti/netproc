
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>   // variable errno
#include <signal.h>  // sigaction
#include <stdio.h>   // putchar
#include <stdlib.h>  // exit
#include <string.h>  // strerror
#include <unistd.h>  // STDIN_FILENO
#include <poll.h>    // poll

#include "terminal.h"
#include "config.h"
#include "packet.h"
#include "rate.h"
#include "process.h"
#include "sock.h"
#include "ring.h"
#include "filter.h"
#include "statistics.h"
// #include "crc32.h"
#include "sufix.h"
#include "timer.h"
#include "show.h"
#include "log.h"
#include "usage.h"
#include "m_error.h"

// a cada vez que o tempo de T_REFRESH segundo(s) é atingido
// esse valor é alterado (entre 0 e 1), para que outras partes, statistics_proc,
// do programa possam ter uma referencia de tempo
#define TIC_TAC( t ) ( ( t ) ? ( t = 0 ) : ( t = 1 ) )

// intervalo de atualização do programa, não alterar
#define T_REFRESH 1

// 1 segundo = 1000 milisegundos
#define TIMEOUT_POLL 1000

static void
sig_handler ( int );

// handled by function sig_handler
// if != 0 program exit
static int prog_exit = 0;

// all resources that needed free on program exit
struct resources_to_free
{
  FILE *log_file;
  log_processes *buff_log;
  size_t len_log;
  process_t *processes;
  uint32_t tot_processes;
  struct ring *ring;
  int sock;
  int exit_status;
};

static void free_resources_and_exit ( struct resources_to_free );

int
main ( int argc, const char **argv )
{
  struct packet packet = {0};
  struct ring ring = {0};
  struct tpacket_block_desc *pbd;
  struct tpacket3_hdr *ppd;
  struct config_op *co;

  process_t *processes = NULL;
  size_t tot_process_act = 0;

  // feature log in file
  FILE *log_file = NULL;
  log_processes *log_file_buffer = NULL;
  size_t len_log_file_buffer = 0;

  int sock;

  bool packtes_reads;
  bool fail_process_pkt;
  int block_num = 0;
  // int tot_blocks = 64; == ring.req.tp_block_nr
  int rp;

  struct sigaction sigact = {.sa_handler = sig_handler};
  sigemptyset ( &sigact.sa_mask );

  sigaction ( SIGINT, &sigact, NULL );
  sigaction ( SIGTERM, &sigact, NULL );

  setup_terminal ();

  co = parse_options ( argc, argv );

  sock = create_socket ( co );

  if ( co->log )
    log_file = setup_log_file ( co );

  create_ring ( sock, &ring );

  // set filter BPF
  set_filter ( sock, co );

  define_sufix ( co );

  setup_ui ( co );
  start_ui ( co );

  const nfds_t nfds = 2;
  struct pollfd poll_set[2] = {
          {.fd = STDIN_FILENO, .events = POLLIN, .revents = 0},
          {.fd = sock, .events = POLLIN | POLLPRI, .revents = 0}};

  // first search by processes
  tot_process_act = get_process_active_con ( &processes, tot_process_act, co );

  // hash_crc32_udp = get_crc32_udp_conection ();

  pbd = ( struct tpacket_block_desc * ) ring.rd[block_num].iov_base;

  double m_timer = start_timer ();

  // main loop
  while ( true )
    {
      packtes_reads = false;
      fail_process_pkt = false;

      while ( pbd->hdr.bh1.block_status & TP_STATUS_USER )
        {
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
              if ( !add_statistics_in_processes (
                           processes, tot_process_act, &packet, co ) )
                {
                  // mark to update processes
                  fail_process_pkt = true;

                  continue;
                }

              packtes_reads = true;
            }

          // pass block controller to kernel
          pbd->hdr.bh1.block_status = TP_STATUS_KERNEL;

          // rotate block
          block_num = ( block_num + 1 ) % ring.req.tp_block_nr;
          pbd = ( struct tpacket_block_desc * ) ring.rd[block_num].iov_base;
        }

      // necessario para zerar contadores quando não ha trafego
      packet.lenght = 0;
      add_statistics_in_processes ( processes, tot_process_act, &packet, co );

      double temp;
      if ( ( temp = timer ( m_timer ) ) >= ( double ) T_REFRESH )
        {
          co->running += temp;
          calc_avg_rate ( processes, tot_process_act, co );

          show_process ( processes, tot_process_act, co );

          if ( co->log )
            log_to_file ( processes,
                          tot_process_act,
                          &log_file_buffer,
                          &len_log_file_buffer,
                          log_file );

          m_timer = restart_timer ();

          TIC_TAC ( co->tic_tac );

          // update processes case necessary
          if ( fail_process_pkt )
            tot_process_act =
                    get_process_active_con ( &processes, tot_process_act, co );
        }

      if ( !packtes_reads )
        {
          rp = poll ( poll_set, nfds, TIMEOUT_POLL );
          if ( rp == -1 )
            {
              // signal event
              if ( errno == EINTR )
                continue;
              else
                fatal_error ( "poll: \"%s\"", strerror ( errno ) );
            }
          else if ( rp > 0 )
            {
              for ( size_t i = 0; i < nfds; i++ )
                {
                  if ( !poll_set[i].revents )
                    continue;

                  if ( poll_set[i].fd == STDIN_FILENO )
                    {
                      if ( running_input ( co ) == P_EXIT )
                        free_resources_and_exit ( ( struct resources_to_free ){
                                .log_file = log_file,
                                .buff_log = log_file_buffer,
                                .len_log = len_log_file_buffer,
                                .sock = sock,
                                .processes = processes,
                                .tot_processes = tot_process_act,
                                .ring = &ring,
                                .exit_status = EXIT_SUCCESS} );
                    }
                }
            }
        }

      // exit signal
      if ( prog_exit )
        free_resources_and_exit (
                ( struct resources_to_free ){.log_file = log_file,
                                             .buff_log = log_file_buffer,
                                             .len_log = len_log_file_buffer,
                                             .sock = sock,
                                             .processes = processes,
                                             .tot_processes = tot_process_act,
                                             .ring = &ring,
                                             .exit_status = prog_exit} );
    }

  return EXIT_SUCCESS;
}

static void
free_resources_and_exit ( struct resources_to_free res )
{
  close_socket ( res.sock );

  free_ring ( res.ring );

  if ( res.log_file )
    free_log ( res.log_file, res.buff_log, res.len_log );

  // free ( res.buff_log_file );

  if ( res.tot_processes )
    free_process ( res.processes, res.tot_processes );

  restore_terminal ();

  exit ( res.exit_status );
}

static void
sig_handler ( int sig )
{
  // "The return value of a simple command is its exit status,
  //  or 128+n if the command is terminated by signal n"
  // by man bash
  prog_exit = 128 + sig;
}
