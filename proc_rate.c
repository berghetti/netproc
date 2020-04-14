
#include <stdlib.h>
#include "process.h"
#include "human_readable.h"


#include <stdio.h> //provisorio

#define Kb (1UL<<10)  // 1024
#define Mb (1UL<<20)  // 1024^2
#define Gb (1UL<<30)  // 1024^3


float calc_Kbps(const size_t bytes);

void
calc_avg_rate(process_t *proc, const size_t tot_proc )
{
  uint64_t sum_bytes_rx,
           sum_bytes_tx,
           sum_pps_rx,
           sum_pps_tx;

  for (size_t i = 0; i < tot_proc; i++)
    {
      sum_bytes_rx = 0;
      sum_bytes_tx = 0;
      sum_pps_rx = 0;
      sum_pps_tx = 0;

      for (size_t j = 0; j < LEN_BUF_CIRC_RATE; j++)
        {
          // printf("pid %d - pps_tx[%d] - %d\n", proc[i].pid, j, proc[i].net_stat.pps_tx[j]);
          sum_bytes_rx += proc[i].net_stat.Bps_rx[j];
          sum_bytes_tx += proc[i].net_stat.Bps_tx[j];

          sum_pps_rx += proc[i].net_stat.pps_rx[j];
          sum_pps_tx += proc[i].net_stat.pps_tx[j];
        }

        // proc[i].net_stat.avg_Bps_rx = (sum_bytes_rx) ? ((sum_bytes_rx) / 1024) / LEN_BUF_CIRC_RATE : 0;
        // proc[i].net_stat.avg_Bps_tx = (sum_bytes_tx) ? ((sum_bytes_tx) / 1024) / LEN_BUF_CIRC_RATE : 0;

        // proc[i].net_stat.avg_Bps_rx = calc_Kbps(sum_bytes_rx);
        // proc[i].net_stat.avg_Bps_tx = calc_Kbps(sum_bytes_tx);

        // printf("sum_bytes_rx - %ld\n", sum_pps_rx);
        // printf("sum_bytes_tx - %ld\n", sum_pps_tx);

        human_readable(proc[i].net_stat.avg_Bps_rx, 10, (sum_bytes_rx / LEN_BUF_CIRC_RATE));
        human_readable(proc[i].net_stat.avg_Bps_tx, 10, (sum_bytes_tx / LEN_BUF_CIRC_RATE));

        // printf("human rx - %s\n", proc[i].net_stat.avg_Bps_rx);
        // printf("human tx - %s\n", proc[i].net_stat.avg_Bps_tx);
        // getchar();

        proc[i].net_stat.avg_pps_rx = (sum_pps_rx) ? (sum_pps_rx / LEN_BUF_CIRC_RATE) : 0;
        proc[i].net_stat.avg_pps_tx = (sum_pps_tx) ? (sum_pps_tx / LEN_BUF_CIRC_RATE) : 0;

    }
}

float calc_Kbps(const size_t bytes)
{
  return (float) (bytes / Kb) / LEN_BUF_CIRC_RATE;
}
