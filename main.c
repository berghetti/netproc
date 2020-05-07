
#include <errno.h>   // variable errno
#include <string.h>  // strerror

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

// a cada vez que o tempo de T_REFRESH segundo(s) é atingido
// esse valor é alterado (entre 0 e 1), para que outras partes, statistics_proc,
// do programa possam ter uma referencia de tempo
#define TIC_TAC( t ) ( ( t ) ? ( t )-- : ( t )++ )

// intervalo de atualização do programa, não alterar
#define T_REFRESH 1.0

// options default
bool udp = true;          // mode TCP
bool view_si = false;     // view in prefix IEC "Kib, Mib, ..."
bool view_bytes = false;  // view in bits
char *iface = NULL;       // sniff all interfaces

uint8_t tic_tac = 0;

process_t *processes = NULL;

int
main ( void )
{
  create_socket ();

  setup_terminal ();

  define_sufix ();

  uint32_t tot_process_act = 0;
  tot_process_act = get_process_active_con ( &processes, tot_process_act );

  uint8_t *buffer = calloc ( IP_MAXPACKET, 1 );
  if ( !buffer )
    fatal_error ( "Error alloc buffer packets: %s", strerror ( errno ) );

  struct sockaddr_ll link_level = {0};
  struct packet packet = {0};

  double m_timer = start_timer ();
  ssize_t bytes;
  // main loop
  while ( 1 )
    {
      bytes = get_packet ( &link_level, buffer, IP_MAXPACKET );

      if ( bytes == -1 )
        {
          free ( buffer );
          close_socket ();
          fatal_error ( "sniffer packets" );
        }

      // se houver dados porem não foi possivel identificar o trafego,
      // não tem estatisticas para ser adicionada aos processos
      if ( bytes > 0 )
        if ( !parse_packet ( &packet, buffer, &link_level ) )
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
