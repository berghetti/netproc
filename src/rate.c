
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

#include <stdlib.h>

#include "processes.h"
#include "round.h"
#include "rate.h"

static void
calc_avg_rate_conection ( process_t *process, const struct config_op *co );

void
calc_avg_rate ( struct processes *processes, const struct config_op *co )
{
  uint64_t sum_bytes_rx, sum_bytes_tx, sum_pps_rx, sum_pps_tx;

  for ( process_t **proc = processes->proc; *proc; proc++ )
    {
      process_t *process = *proc;

      sum_bytes_rx = 0;
      sum_bytes_tx = 0;
      sum_pps_rx = 0;
      sum_pps_tx = 0;

      // soma todos os bytes e pacotes recebidos do processo
      for ( int j = 0; j < LEN_BUF_CIRC_RATE; j++ )
        {
          sum_bytes_rx += process->net_stat.Bps_rx[j];
          sum_bytes_tx += process->net_stat.Bps_tx[j];

          sum_pps_rx += process->net_stat.pps_rx[j];
          sum_pps_tx += process->net_stat.pps_tx[j];
        }

      // transforma bytes em bits
      if ( !co->view_bytes )
        {
          sum_bytes_rx *= 8;
          sum_bytes_tx *= 8;
        }

      // calcula a média de bytes recebidos
      process->net_stat.avg_Bps_rx =
              ( sum_bytes_rx ) ? m_round ( ( double ) ( sum_bytes_rx ) /
                                           LEN_BUF_CIRC_RATE )
                               : 0;

      // calcula a média de bytes enviados
      process->net_stat.avg_Bps_tx =
              ( sum_bytes_tx ) ? m_round ( ( double ) ( sum_bytes_tx ) /
                                           LEN_BUF_CIRC_RATE )
                               : 0;

      // calcula a média de pacotes recebidos
      process->net_stat.avg_pps_rx =
              ( sum_pps_rx )
                      ? m_round ( ( double ) sum_pps_rx / LEN_BUF_CIRC_RATE )
                      : 0;

      process->net_stat.avg_pps_tx =
              ( sum_pps_tx )
                      ? m_round ( ( double ) sum_pps_tx / LEN_BUF_CIRC_RATE )
                      : 0;

      // calcula taxa individual de cada conexão
      if ( co->view_conections )
        calc_avg_rate_conection ( process, co );
    }
}

static void
calc_avg_rate_conection ( process_t *process, const struct config_op *co )
{
  uint64_t sum_bytes_rx, sum_bytes_tx, sum_pps_rx, sum_pps_tx;

  for ( size_t i = 0; i < process->total_conections; i++ )
    {
      sum_bytes_rx = 0;
      sum_bytes_tx = 0;
      sum_pps_rx = 0;
      sum_pps_tx = 0;

      for ( int j = 0; j < LEN_BUF_CIRC_RATE; j++ )
        {
          sum_bytes_rx += process->conection[i].net_stat.Bps_rx[j];
          sum_bytes_tx += process->conection[i].net_stat.Bps_tx[j];

          sum_pps_rx += process->conection[i].net_stat.pps_rx[j];
          sum_pps_tx += process->conection[i].net_stat.pps_tx[j];
        }

      // transforma bytes em bits
      if ( !co->view_bytes )
        {
          sum_bytes_rx *= 8;
          sum_bytes_tx *= 8;
        }

      // calcula a média de bytes recebidos
      process->conection[i].net_stat.avg_Bps_rx =
              ( sum_bytes_rx ) ? m_round ( ( double ) ( sum_bytes_rx ) /
                                           LEN_BUF_CIRC_RATE )
                               : 0;

      process->conection[i].net_stat.avg_Bps_tx =
              ( sum_bytes_tx ) ? m_round ( ( double ) ( sum_bytes_tx ) /
                                           LEN_BUF_CIRC_RATE )
                               : 0;

      // calcula a média de pacotes recebidos
      process->conection[i].net_stat.avg_pps_rx =
              ( sum_pps_rx )
                      ? m_round ( ( double ) sum_pps_rx / LEN_BUF_CIRC_RATE )
                      : 0;

      process->conection[i].net_stat.avg_pps_tx =
              ( sum_pps_tx )
                      ? m_round ( ( double ) sum_pps_tx / LEN_BUF_CIRC_RATE )
                      : 0;
    }
}
