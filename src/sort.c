
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

#define _GNU_SOURCE  // qsort_r
#include <stdlib.h>
#include <stdint.h>

#include "processes.h"
#include "conection.h"
#include "sort.h"

static int
compara_processo ( const void *restrict p1,
                   const void *restrict p2,
                   void *restrict mode )
{
  process_t *proc1 = *( process_t ** ) p1;
  process_t *proc2 = *( process_t ** ) p2;

  int64_t r;
  switch ( *( int * ) mode )
    {
      case RATE_RX:
        r = proc2->net_stat.avg_Bps_rx - proc1->net_stat.avg_Bps_rx;
        break;
      case RATE_TX:
        r = proc2->net_stat.avg_Bps_tx - proc1->net_stat.avg_Bps_tx;
        break;
      case TOT_RX:
        r = proc2->net_stat.tot_Bps_rx - proc1->net_stat.tot_Bps_rx;
        break;
      case TOT_TX:
        r = proc2->net_stat.tot_Bps_tx - proc1->net_stat.tot_Bps_tx;
        break;
      case PPS_RX:
        r = proc2->net_stat.avg_pps_rx - proc1->net_stat.avg_pps_rx;
        break;
      case PPS_TX:
        r = proc2->net_stat.avg_pps_tx - proc1->net_stat.avg_pps_tx;
        break;

      // PID
      default:
        r = proc1->pid - proc2->pid;
    }

  if ( r > 0 )
    return 1;
  else if ( r < 0 )
    return -1;
  else
    return 0;
}

static int
compara_conexao ( const void *restrict p1,
                  const void *restrict p2,
                  void *restrict mode )
{
  conection_t *con1 = (conection_t *) p1;
  conection_t *con2 = (conection_t *) p2;

  int64_t r;
  switch ( *( int * ) mode )
    {
      case RATE_TX:
        r = con2->net_stat.avg_Bps_tx - con1->net_stat.avg_Bps_tx;
        break;
      case PPS_RX:
        r = con2->net_stat.avg_pps_rx - con1->net_stat.avg_pps_rx;
        break;
      case PPS_TX:
        r = con2->net_stat.avg_pps_tx - con1->net_stat.avg_pps_tx;
        break;

      // RATE RX
      default:
        r = con2->net_stat.avg_Bps_rx - con1->net_stat.avg_Bps_rx;
    }
  if ( r > 0 )
    return 1;
  else if ( r < 0 )
    return -1;
  else
    return 0;
}

void
sort ( process_t **proc,
       size_t tot_process,
       int mode,
       const struct config_op *co )
{
  qsort_r ( proc,
            tot_process,
            sizeof ( process_t * ),
            compara_processo,
            ( void * ) &mode );

  if ( co->view_conections )
    for ( size_t i = 0; i < tot_process; i++ )
      qsort_r ( proc[i]->conection,
                proc[i]->total_conections,
                sizeof ( conection_t ),
                compara_conexao,
                ( void * ) &mode );
}
