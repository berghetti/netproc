
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
#include <term.h>    // tputs

#include "m_error.h"
#include "network.h"
#include "proc_rate.h"
#include "process.h"
#include "show.h"
#include "sock.h"
#include "statistics_proc.h"
#include "sufix.h"
#include "terminal.h"
#include "timer.h"
#include "usage.h"

// a cada vez que o tempo de T_REFRESH segundo(s) é atingido
// esse valor é alterado (entre 0 e 1), para que outras partes, statistics_proc,
// do programa possam ter uma referencia de tempo
#define TIC_TAC( t ) ( ( t ) ? ( t )-- : ( t )++ )

// intervalo de atualização do programa, não alterar
#define T_REFRESH 1.0

static void
clear_exit ( void );

static void
sig_handler ( int );

static void
parse_options ( int, const char ** );

// options default
bool udp = false;         // mode TCP
bool view_si = false;     // view in prefix IEC "Kib, Mib, ..."
bool view_bytes = false;  // view in bits
char *iface = NULL;       // sniff all interfaces

uint8_t *buff_pkt = NULL;
process_t *processes = NULL;
static uint32_t tot_process_act = 0;
uint8_t tic_tac = 0;

int
main ( int argc, const char **argv )
{
  parse_options ( argc, argv );

  atexit ( clear_exit );

  setup_terminal ();

  create_socket ();

  define_sufix ();

  struct sigaction sigact = {.sa_handler = sig_handler};
  sigemptyset ( &sigact.sa_mask );

  sigaction ( SIGINT, &sigact, NULL );
  sigaction ( SIGTERM, &sigact, NULL );

  // enquanto não encontrar processos com conexões ativas
  // testar isso...
  while ( 0 == ( tot_process_act = get_process_active_con (
                         &processes, tot_process_act ) ) )
    printf ( "\rNenhum processo com conexão ativa encontrado, procurando..." );

  buff_pkt = calloc ( IP_MAXPACKET, 1 );
  if ( !buff_pkt )
    fatal_error ( "Error alloc buff_pkt packets: %s", strerror ( errno ) );

  struct sockaddr_ll link_level = {0};
  struct packet packet = {0};

  double m_timer = start_timer ();
  ssize_t bytes;

  // main loop
  while ( 1 )
    {
      bytes = get_packet ( &link_level, buff_pkt, IP_MAXPACKET );

      if ( bytes == -1 )
        fatal_error ( "sniffer packets" );

      // se houver dados porem não foi possivel identificar o trafego,
      // não tem estatisticas para ser adicionada aos processos.
      // deve ser trafego de protocolo não suportado
      if ( bytes > 0 )
        if ( !parse_packet ( &packet, buff_pkt, &link_level ) )
          goto PRINT;

      packet.lenght = bytes;

      // se não for possivel identificar de qual processo o trafego pertence
      // é sinal que existe um novo processo que ainda não foi mapeado,
      // então atualizamos a lista de processos com conexões ativas.

      // mesmo que não tenha dados para atualizar(bytes == 0), chamamos a
      // função para que possa contabilizar na média.
      if ( !add_statistics_in_processes (
                   processes, tot_process_act, &packet ) )
        if ( bytes > 0 )
          tot_process_act =
                  get_process_active_con ( &processes, tot_process_act );

    PRINT:
      if ( timer ( m_timer ) >= T_REFRESH )
        {
          calc_avg_rate ( processes, tot_process_act );
          show_process ( processes, tot_process_act );

          m_timer = restart_timer ();

          TIC_TAC ( tic_tac );
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
                break;
              case 'B':
                view_bytes = true;
                break;
              case 's':
                if ( *( *argv + 2 ) && *( *argv + 2 ) == 'i' )
                  view_si = true;
                break;
              case 'h':
                usage ();
                exit ( EXIT_SUCCESS );
              case 'v':
                show_version ();
                exit ( EXIT_SUCCESS );
              default:
                usage ();
                exit ( EXIT_FAILURE );
            }
        }
    }
}

static void
clear_exit ( void )
{
  close_socket ();

  if ( buff_pkt )
    free ( buff_pkt );

  if ( tot_process_act )
    free_process ( processes, tot_process_act );

  restore_terminal ();
}

static void
sig_handler ( int sig )
{
  // The return value of a simple command is its exit status,
  // or 128+n if the command is terminated by signal n
  // by man bash
  exit ( 128 + sig );
}
