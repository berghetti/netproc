
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

#include "process.h"
#include "conection.h"
#include "sort.h"

static int
compara_processo ( const void *restrict p1,
                   const void *restrict p2,
                   void *restrict mode )
{
  int64_t r;

  switch ( *( int * ) mode )
    {
      case RATE_RX:
        r = ( ( process_t * ) p2 )->net_stat.avg_Bps_rx -
            ( ( process_t * ) p1 )->net_stat.avg_Bps_rx;
        break;
      case RATE_TX:
        r = ( ( process_t * ) p2 )->net_stat.avg_Bps_tx -
            ( ( process_t * ) p1 )->net_stat.avg_Bps_tx;
        break;
      case TOT_RX:
        r = ( ( process_t * ) p2 )->net_stat.tot_Bps_rx -
            ( ( process_t * ) p1 )->net_stat.tot_Bps_rx;
        break;
      case TOT_TX:
        r = ( ( process_t * ) p2 )->net_stat.tot_Bps_tx -
            ( ( process_t * ) p1 )->net_stat.tot_Bps_tx;
        break;
      case PPS_RX:
        r = ( ( process_t * ) p2 )->net_stat.avg_pps_rx -
            ( ( process_t * ) p1 )->net_stat.avg_pps_rx;
        break;
      case PPS_TX:
        r = ( ( process_t * ) p2 )->net_stat.avg_pps_tx -
            ( ( process_t * ) p1 )->net_stat.avg_pps_tx;
        break;

      default:
        // ordena menor para maior
        r = ( ( process_t * ) p1 )->pid - ( ( process_t * ) p2 )->pid;
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
  int64_t r;
  switch ( *( int * ) mode )
    {
      case RATE_TX:
        r = ( ( conection_t * ) p2 )->net_stat.avg_Bps_tx -
            ( ( conection_t * ) p1 )->net_stat.avg_Bps_tx;
        break;
      case PPS_RX:
        r = ( ( conection_t * ) p2 )->net_stat.avg_pps_rx -
            ( ( conection_t * ) p1 )->net_stat.avg_pps_rx;
        break;
      case PPS_TX:
        r = ( ( conection_t * ) p2 )->net_stat.avg_pps_tx -
            ( ( conection_t * ) p1 )->net_stat.avg_pps_tx;
        break;

      default:
        r = ( ( conection_t * ) p2 )->net_stat.avg_Bps_rx -
            ( ( conection_t * ) p1 )->net_stat.avg_Bps_rx;
    }
  if ( r > 0 )
    return 1;
  else if ( r < 0 )
    return -1;
  else
    return 0;
}

void
sort ( process_t *restrict proc,
       int tot_process,
       int mode,
       const struct config_op *restrict co )
{
  qsort_r ( proc,
            tot_process,
            sizeof ( process_t ),
            compara_processo,
            ( void * ) &mode );

  if ( co->view_conections )
    for ( int i = 0; i < tot_process; i++ )
      qsort_r ( proc[i].conection,
                proc[i].total_conections,
                sizeof ( conection_t ),
                compara_conexao,
                ( void * ) &mode );
}
