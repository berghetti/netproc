
#include <stdio.h>

#include "process.h"
#include "terminal.h"  // clear_cmd

void
show_process ( const process_t *const processes, const size_t tot_process )
{
  // limpa a tela e o scrollback
  clear_cmd ();

  printf ( "%-5s\t %-45s %s\t %s\t %-14s\t %s \n",
           "PID",
           "PROGRAM",
           "PPS TX",
           "PPS RX",
           "RATE UP",
           "RATE DOWN" );

  for ( size_t i = 0; i < tot_process; i++ )
    {
      // só exibe o processo se tiver fluxo de rede
      if ( processes[i].net_stat.avg_Bps_rx ||
           processes[i].net_stat.avg_Bps_tx ||
           processes[i].net_stat.avg_pps_rx ||
           processes[i].net_stat.avg_pps_tx )
        {
          printf ( "%-5d\t %-45s %ld\t %ld\t %-14s\t %s\t \n",
                   processes[i].pid,
                   processes[i].name,
                   processes[i].net_stat.avg_pps_tx,
                   processes[i].net_stat.avg_pps_rx,
                   processes[i].net_stat.tx_rate,
                   processes[i].net_stat.rx_rate );
        }
    }
}
