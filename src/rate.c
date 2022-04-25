
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

  // sum all bytes and packets received and sent
  for ( int i = 0; i < SAMPLE_SPACE_SIZE; i++ )
    {
      sum_bytes_rx += ns->Bps_rx[i];
      sum_bytes_tx += ns->Bps_tx[i];

      sum_pps_rx += ns->pps_rx[i];
      sum_pps_tx += ns->pps_tx[i];
    }

  // transform bytes to bits
  if ( !view_bytes )
    {
      sum_bytes_rx *= 8;
      sum_bytes_tx *= 8;
    }

  // calc averege of bytes received / sent
  ns->avg_Bps_rx = m_round ( ( double ) sum_bytes_rx / SAMPLE_SPACE_SIZE );
  ns->avg_Bps_tx = m_round ( ( double ) sum_bytes_tx / SAMPLE_SPACE_SIZE );

  // calc averege of packets received / sent
  ns->avg_pps_rx = m_round ( ( double ) sum_pps_rx / SAMPLE_SPACE_SIZE );
  ns->avg_pps_tx = m_round ( ( double ) sum_pps_tx / SAMPLE_SPACE_SIZE );
}

void
rate_calc ( struct processes *processes, const struct config_op *co )
{
  for ( size_t i = 0; i < processes->total; i++ )
    {
      process_t *process = processes->proc[i];

      rate_net_stat ( &process->net_stat, co->view_bytes );

      // calc rate to each connection
      if ( co->view_conections )
        {
          for ( size_t i = 0; i < process->total_conections; i++ )
            rate_net_stat ( &process->conections[i]->net_stat, co->view_bytes );
        }
    }
}

#define UPDATE_ID_BUFF( id ) ( ( id ) = ( ( id ) + 1 ) % SAMPLE_SPACE_SIZE )

static uint8_t idx_cir = 0;

void
rate_add_rx ( struct net_stat *ns, size_t lenght )
{
  ns->pps_rx[idx_cir]++;
  ns->Bps_rx[idx_cir] += lenght;

  ns->bytes_last_sec_rx += lenght;
  ns->tot_Bps_rx += lenght;
}

void
rate_add_tx ( struct net_stat *ns, size_t lenght )
{
  ns->pps_tx[idx_cir]++;
  ns->Bps_tx[idx_cir] += lenght;

  ns->bytes_last_sec_tx += lenght;
  ns->tot_Bps_tx += lenght;
}

void
rate_update ( struct processes *processes, const struct config_op *co )
{
  UPDATE_ID_BUFF ( idx_cir );

  for ( size_t i = 0; i < processes->total; i++ )
    {
      process_t *process = processes->proc[i];

      process->net_stat.Bps_rx[idx_cir] = 0;
      process->net_stat.Bps_tx[idx_cir] = 0;
      process->net_stat.pps_rx[idx_cir] = 0;
      process->net_stat.pps_tx[idx_cir] = 0;

      process->net_stat.bytes_last_sec_rx = 0;
      process->net_stat.bytes_last_sec_tx = 0;

      // clear statistics of each connection alsa
      if ( co->view_conections )
        {
          for ( size_t c = 0; c < process->total_conections; c++ )
            {
              process->conections[c]->net_stat.Bps_rx[idx_cir] = 0;
              process->conections[c]->net_stat.Bps_tx[idx_cir] = 0;
              process->conections[c]->net_stat.pps_rx[idx_cir] = 0;
              process->conections[c]->net_stat.pps_tx[idx_cir] = 0;
            }
        }
    }
}
