
#include <stdlib.h>
#include "process.h"


void
calc_avg_rate(process_t *proc, const size_t tot_proc )
{
  size_t sum_Bps_rx,
         sum_Bps_tx,
         sum_pps_rx,
         sum_pps_tx;

  for (size_t i = 0; i < tot_proc; i++)
    {
      sum_Bps_rx = 0;
      sum_Bps_tx = 0;
      sum_pps_rx = 0;
      sum_pps_tx = 0;

      for (size_t j = 0; j < LEN_BUF_CIRC_RATE; j++)
        {
          // printf("pid %d - pps_tx[%d] - %d\n", proc[i].pid, j, proc[i].net_stat.pps_tx[j]);
          sum_Bps_rx += proc[i].net_stat.Bps_rx[j];
          sum_Bps_tx += proc[i].net_stat.Bps_tx[j];

          sum_pps_rx += proc[i].net_stat.pps_rx[j];
          sum_pps_tx += proc[i].net_stat.pps_tx[j];
        }

        proc[i].net_stat.avg_Bps_rx = (sum_Bps_rx) ? ((sum_Bps_rx) / 1024) / LEN_BUF_CIRC_RATE : 0;
        proc[i].net_stat.avg_Bps_tx = (sum_Bps_tx) ? ((sum_Bps_tx) / 1024) / LEN_BUF_CIRC_RATE : 0;

        proc[i].net_stat.avg_pps_rx = (sum_pps_rx) ? (sum_pps_rx / LEN_BUF_CIRC_RATE) : 0;
        proc[i].net_stat.avg_pps_tx = (sum_pps_tx) ? (sum_pps_tx / LEN_BUF_CIRC_RATE) : 0;

    }
}
