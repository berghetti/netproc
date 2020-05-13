
#include <stdio.h>

#include "process.h"
#include "terminal.h"  // clear_cmd

// espaçamento entre as colunas
#define PID     -5
#define PROGRAM -45
#define PPS     -8
#define RATE    -14

void
show_process ( const process_t *const processes, const size_t tot_process )
{
  // limpa a tela e o scrollback
  clear_cmd ();

  printf ( "%*s %*s %*s %*s %*s %s\n",
           PID,
           "PID",
           PROGRAM,
           "PROGRAM",
           PPS,
           "PPS TX",
           PPS,
           "PPS RX",
           RATE,
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
          printf ( "%*d %*s %*ld %*ld %*s %s\n",
                   PID,
                   processes[i].pid,
                   PROGRAM,
                   processes[i].name,
                   PPS,
                   processes[i].net_stat.avg_pps_tx,
                   PPS,
                   processes[i].net_stat.avg_pps_rx,
                   RATE,
                   processes[i].net_stat.tx_rate,
                   processes[i].net_stat.rx_rate );
        }
    }
}
