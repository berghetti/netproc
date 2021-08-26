
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
#include <stdbool.h>

#include "processes.h"
#include "round.h"
#include "rate.h"

static void
rate_net_stat ( struct net_stat *ns, bool view_bytes )
{
  uint64_t sum_bytes_rx = 0, sum_bytes_tx = 0, sum_pps_rx = 0, sum_pps_tx = 0;

  // soma todos os bytes e pacotes recebidos do processo
  for ( int i = 0; i < SAMPLE_SPACE_SIZE; i++ )
    {
      sum_bytes_rx += ns->Bps_rx[i];
      sum_bytes_tx += ns->Bps_tx[i];

      sum_pps_rx += ns->pps_rx[i];
      sum_pps_tx += ns->pps_tx[i];
    }

  // bytes to bits
  if ( !view_bytes )
    {
      sum_bytes_rx *= 8;
      sum_bytes_tx *= 8;
    }

  // calcula a média de bytes recebidos /  enviados
  ns->avg_Bps_rx = m_round ( ( double ) sum_bytes_rx / SAMPLE_SPACE_SIZE );
  ns->avg_Bps_tx = m_round ( ( double ) sum_bytes_tx / SAMPLE_SPACE_SIZE );

  // calcula a média de pacotes recebidos / enviados
  ns->avg_pps_rx = m_round ( ( double ) sum_pps_rx / SAMPLE_SPACE_SIZE );
  ns->avg_pps_tx = m_round ( ( double ) sum_pps_tx / SAMPLE_SPACE_SIZE );
}

void
rate_calc ( struct processes *processes, const struct config_op *co )
{
  for ( process_t **proc = processes->proc; *proc; proc++ )
    {
      process_t *process = *proc;

      rate_net_stat ( &process->net_stat, co->view_bytes );

      // calcula taxa individual de cada conexão
      if ( co->view_conections )
        {
          for ( size_t i = 0; i < process->total_conections; i++ )
            rate_net_stat ( &process->conection[i].net_stat, co->view_bytes );
        }
    }
}
