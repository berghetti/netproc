
#include <stdio.h>

#include "process.h"
#include "clear.h"    // clear_cmd


// inline static void cls(void);

void
show_process(const process_t *const processes, const size_t tot_process)
{
  // limpa a tela e o scroll
  clear_cmd(1);

  printf("%-5s\t %-45s %s\t %s\t %-14s\t %s \n",
        "PID", "PROGRAM", "PPS TX", "PPS RX", "RATE UP", "RATE DOWN");

  for (size_t i = 0; i < tot_process; i++)
    {
      printf("%-5d\t %-45s %ld\t %ld\t %-14s\t %s\t \n",
            processes[i].pid,
            processes[i].name,
            processes[i].net_stat.avg_pps_tx,
            processes[i].net_stat.avg_pps_rx,
            processes[i].net_stat.tx_rate,
            processes[i].net_stat.rx_rate);
    }
}

// limpa tela
// inline static void cls(void)
// {
//   // putp(clear_screen);                 // limpa tela
//   // putp(reset_1string);                // reset terminal, exclui scroll
//   // putp(tparm(cursor_address, 0, 0));  // posiciona cursor para 0 linha coluna 0
//
//    // versão com codigo de escape ANSI direto
//    // printf("\033[2J"     // limpa a parte visivel da tela
//    //        "\033[3J"     // limpa o buffer do scroll
//    //        "\033[0;0H"); // posiciona o cursor para a linha 0, coluna 0
// }
