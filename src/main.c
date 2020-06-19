
/*
 *  Copyright (C) 2020 Mayco S. Berghetti
 *
 *
 *  This program is free software: you can redistribute it and/or modify
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

#include "crc32.h"
#include "packet.h"
#include "rate.h"
#include "process.h"
#include "show.h"
#include "sock.h"
#include "ring.h"
#include "filter.h"
#include "statistics.h"
#include "sufix.h"
#include "terminal.h"
#include "timer.h"
#include "usage.h"
#include "m_error.h"

// a cada vez que o tempo de T_REFRESH segundo(s) é atingido
// esse valor é alterado (entre 0 e 1), para que outras partes, statistics_proc,
// do programa possam ter uma referencia de tempo
#define TIC_TAC( t ) ( ( t ) ? ( t )-- : ( t )++ )

// intervalo de atualização do programa, não alterar
#define T_REFRESH 1

// 1 segundo = 1000 milisegundos
#define TIMEOUT_POLL 1000

static void
clear_exit ( void );

static void
sig_handler ( int );

static void
parse_options ( int argc, const char **argv );

// options default
bool udp = false;               // mode TCP
bool view_si = false;           // view in prefix IEC "Kib, Mib, ..."
bool view_bytes = false;        // view in bits
bool view_conections = false;   // show process conections
bool translate_host = true;     // translate ip in name using DNS
bool translate_service = true;  // translate service, ex 443 -> https
hash_t hash_crc32_udp = 0;      // para checar se arquivo foi alterado
char *iface = NULL;             // sniff all interfaces

// uint8_t *buff_pkt = NULL;
static process_t *processes = NULL;
static uint32_t tot_process_act = 0;
uint8_t tic_tac = 0;

static int sock;
static struct ring ring;

int
main ( int argc, const char **argv )
{
  // struct ring ring;
  struct packet packet = {0};
  struct tpacket_block_desc *pbd;
  struct tpacket3_hdr *ppd;
  bool packtes_reads;
  int block_num = 0;
  // int tot_blocks = 64; == ring.req.tp_block_nr
  int rp;
  hash_t hash_tmp;

  atexit ( clear_exit );

  setup_terminal ();

  parse_options ( argc, argv );

  sock = create_socket ();

  create_ring ( sock, &ring );

  set_filter ( sock );

  define_sufix ();

  struct sigaction sigact = {.sa_handler = sig_handler};
  sigemptyset ( &sigact.sa_mask );

  sigaction ( SIGINT, &sigact, NULL );
  sigaction ( SIGTERM, &sigact, NULL );

  const nfds_t nfds = 2;
  struct pollfd poll_set[2] = {
          {.fd = STDIN_FILENO, .events = POLLIN, .revents = 0},
          {.fd = sock, .events = POLLIN | POLLPRI, .revents = 0}};

  // first search by processes
  tot_process_act = get_process_active_con ( &processes, tot_process_act );

  hash_crc32_udp = get_crc32_udp_conection ();

  init_ui ();  // setup
  start_ui ();

  pbd = ( struct tpacket_block_desc * ) ring.rd[block_num].iov_base;

  double m_timer = start_timer ();
  // main loop
  while ( 1 )
    {
      packtes_reads = false;
      while ( pbd->hdr.bh1.block_status & TP_STATUS_USER )
        {
          ppd = ( struct tpacket3_hdr * ) ( ( uint8_t * ) pbd +
                                            pbd->hdr.bh1.offset_to_first_pkt );

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
              // atualizamos a lista de processos com conexões ativas.
              if ( !add_statistics_in_processes (
                           processes, tot_process_act, &packet ) )
                {
                  // mas antes de atualizar a lista de processos, checamos
                  // se é um pacote udp e se as conexões UDP
                  // teveram alterações desde a ultima checagem, caso
                  // não tenha alteração, nao tem porque atualizar a lista de
                  // processos, que é um processo caro.
                  // isso é necessario porque alguns aplicavos não mantem uma
                  // conexão UDP por tempo suficiente para o kernel listar
                  if ( packet.protocol == IPPROTO_UDP )
                    {
                      hash_tmp = get_crc32_udp_conection ();
                      // fprintf ( stderr, "hash - %lx\n", hash_tmp );
                      if ( hash_crc32_udp == hash_tmp )
                        continue;

                      hash_crc32_udp = hash_tmp;
                    }

                  // fprintf ( stderr,
                  //           "%d:%d <-> %d:%d\n",
                  //           packet.local_address,
                  //           packet.local_port,
                  //           packet.remote_address,
                  //           packet.remote_port );
                  tot_process_act = get_process_active_con ( &processes,
                                                             tot_process_act );

                  continue;
                }

              packtes_reads = true;
            }

          // flush block
          pbd->hdr.bh1.block_status = TP_STATUS_KERNEL;

          // rotete block
          block_num = ( block_num + 1 ) % ring.req.tp_block_nr;
          pbd = ( struct tpacket_block_desc * ) ring.rd[block_num].iov_base;
        }

      // necessario para zerar contadores quando não ha trafego
      packet.lenght = 0;
      add_statistics_in_processes ( processes, tot_process_act, &packet );

      if ( timer ( m_timer ) >= ( double ) T_REFRESH )
        {
          calc_avg_rate ( processes, tot_process_act );

          show_process ( processes, tot_process_act );

          m_timer = restart_timer ();

          TIC_TAC ( tic_tac );
        }

      if ( !packtes_reads )
        {
          rp = poll ( poll_set, nfds, TIMEOUT_POLL );
          if ( rp == -1 )
            fatal_error ( "poll %s", strerror ( errno ) );
          if ( rp > 0 )
            {
              for ( size_t i = 0; i < nfds; i++ )
                {
                  if ( !poll_set[i].revents )
                    continue;

                  if ( poll_set[i].fd == STDIN_FILENO )
                    running_input ();
                }
            }
        }
    }

  return EXIT_SUCCESS;
}

static void
parse_options ( int argc, const char **argv )
{
  // parse options
  while ( --argc )
    {
      if ( *++argv && **argv == '-' )
        {
          switch ( *( *argv + 1 ) )
            {
              case 'u':
                udp = true;
                break;
              case 'i':
                iface = ( char * ) *++argv;
                argc--;
                break;
              case 'B':
                view_bytes = true;
                break;
              case 'c':
                view_conections = true;
                break;
              case 's':
                if ( *( *argv + 2 ) && *( *argv + 2 ) == 'i' )
                  {
                    view_si = true;
                    break;
                  }
                goto FAIL;
              case 'n':
                view_conections = true;

                if ( *( *argv + 2 ) && *( *argv + 2 ) == 'h' )
                  {
                    translate_host = false;
                    break;
                  }
                else if ( *( *argv + 2 ) && *( *argv + 2 ) == 'p' )
                  {
                    translate_service = false;
                    break;
                  }
                else if ( ( *( *argv + 2 ) ) )
                  goto FAIL;

                translate_host = translate_service = false;
                break;
              case 'h':
                usage ();
                exit ( EXIT_SUCCESS );
              case 'v':
                show_version ();
                exit ( EXIT_SUCCESS );
              default:
                goto FAIL;
            }
        }
      else
        goto FAIL;
    }

  return;

FAIL:
  error ( "Invalid argument '%s'", *argv );
  usage ();
  exit ( EXIT_FAILURE );
}

static void
clear_exit ( void )
{
  close_socket ( sock );

  free_ring ( &ring );

  if ( tot_process_act )
    free_process ( processes, tot_process_act );

  restore_terminal ();
}

static void
sig_handler ( int sig )
{
  // "The return value of a simple command is its exit status,
  //  or 128+n if the command is terminated by signal n"
  // by man bash
  exit ( 128 + sig );
}
