
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

#include <stdlib.h>

#include "human_readable.h"
#include "process.h"
#include "round.h"

// defined in main.c
extern bool view_bytes;

void
calc_avg_rate ( process_t *proc, const size_t tot_proc )
{
  uint64_t sum_bytes_rx, sum_bytes_tx, sum_pps_rx, sum_pps_tx;

  // percorre todos os processos...
  for ( size_t i = 0; i < tot_proc; i++ )
    {
      sum_bytes_rx = 0;
      sum_bytes_tx = 0;
      sum_pps_rx = 0;
      sum_pps_tx = 0;

      // ... e soma todos os bytes e pacotes recebidos
      for ( size_t j = 0; j < LEN_BUF_CIRC_RATE; j++ )
        {
          sum_bytes_rx += proc[i].net_stat.Bps_rx[j];
          sum_bytes_tx += proc[i].net_stat.Bps_tx[j];

          sum_pps_rx += proc[i].net_stat.pps_rx[j];
          sum_pps_tx += proc[i].net_stat.pps_tx[j];
        }

      // transforma bytes em bits
      if ( !view_bytes )
        {
          sum_bytes_rx *= 8;
          sum_bytes_tx *= 8;
        }

      // calcula a média de bytes recebidos
      proc[i].net_stat.avg_Bps_rx =
              ( sum_bytes_rx ) ? m_round ( ( double ) ( sum_bytes_rx ) /
                                           LEN_BUF_CIRC_RATE )
                               : 0;

      proc[i].net_stat.avg_Bps_tx =
              ( sum_bytes_tx ) ? m_round ( ( double ) ( sum_bytes_tx ) /
                                           LEN_BUF_CIRC_RATE )
                               : 0;

      // calcula a média de pacotes recebidos
      proc[i].net_stat.avg_pps_rx =
              ( sum_pps_rx )
                      ? m_round ( ( double ) sum_pps_rx / LEN_BUF_CIRC_RATE )
                      : 0;

      proc[i].net_stat.avg_pps_tx =
              ( sum_pps_tx )
                      ? m_round ( ( double ) sum_pps_tx / LEN_BUF_CIRC_RATE )
                      : 0;

      // transforma o total de bytes recebidos em
      // algo legivel, como 1024 bits em 1 Kib
      human_readable ( proc[i].net_stat.rx_rate,
                       LEN_STR_RATE,
                       proc[i].net_stat.avg_Bps_rx );

      human_readable ( proc[i].net_stat.tx_rate,
                       LEN_STR_RATE,
                       proc[i].net_stat.avg_Bps_tx );
    }
}
