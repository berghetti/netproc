
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
