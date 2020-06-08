
#define _GNU_SOURCE  // qsort_r
#include <stdlib.h>

#include "process.h"
#include "conection.h"
#include "sort.h"

int
compara_processo ( const void *restrict p1,
                   const void *restrict p2,
                   void *restrict arg )
{
  // fprintf(stderr, "%d\n", *(int *)arg);
  switch ( *( int * ) arg )
    {
      case RATE_RX:
        return ( ( process_t * ) p2 )->net_stat.avg_Bps_rx -
               ( ( process_t * ) p1 )->net_stat.avg_Bps_rx;
        break;
      case RATE_TX:
        return ( ( process_t * ) p2 )->net_stat.avg_Bps_tx -
               ( ( process_t * ) p1 )->net_stat.avg_Bps_tx;
        break;
      case TOT_RX:
        return ( ( process_t * ) p2 )->net_stat.tot_Bps_rx -
               ( ( process_t * ) p1 )->net_stat.tot_Bps_rx;
        break;
      case TOT_TX:
        return ( ( process_t * ) p2 )->net_stat.tot_Bps_tx -
               ( ( process_t * ) p1 )->net_stat.tot_Bps_tx;
        break;
      case PPS_RX:
        return ( ( process_t * ) p2 )->net_stat.avg_pps_rx -
               ( ( process_t * ) p1 )->net_stat.avg_pps_rx;
        break;
      case PPS_TX:
        return ( ( process_t * ) p2 )->net_stat.avg_pps_tx -
               ( ( process_t * ) p1 )->net_stat.avg_pps_tx;
        break;

      default:
        // ordena menor para maior
        return ( ( process_t * ) p1 )->pid - ( ( process_t * ) p2 )->pid;
    }
}

int
compara_conexao ( const void *restrict p1,
                  const void *restrict p2,
                  void *restrict arg )
{
  switch ( *( int * ) arg )
    {
      case RATE_TX:
        return ( ( conection_t * ) p2 )->net_stat.avg_Bps_tx -
               ( ( conection_t * ) p1 )->net_stat.avg_Bps_tx;
        break;
      case PPS_RX:
        return ( ( conection_t * ) p2 )->net_stat.avg_pps_rx -
               ( ( conection_t * ) p1 )->net_stat.avg_pps_rx;
        break;
      case PPS_TX:
        return ( ( conection_t * ) p2 )->net_stat.avg_pps_tx -
               ( ( conection_t * ) p1 )->net_stat.avg_pps_tx;
        break;

      default:
        return ( ( conection_t * ) p2 )->net_stat.avg_Bps_rx -
               ( ( conection_t * ) p1 )->net_stat.avg_Bps_rx;
    }
}

void
sort ( process_t *proc, int tot_process, int mode )
{
  qsort_r ( proc,
            tot_process,
            sizeof ( process_t ),
            compara_processo,
            ( void * ) &mode );

  for ( int i = 0; i < tot_process; i++ )
    qsort_r ( proc[i].conection,
              proc[i].total_conections,
              sizeof ( conection_t ),
              compara_conexao,
              ( void * ) &mode );
}
