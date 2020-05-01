
#include <stdbool.h>
#include <stdio.h>    // provisorio

#include "process.h"
#include "network.h"

// incremento circular de 0 até LEN_BUF_CIRC_RATE - 1
#define UPDATE_ID_BUFF(id) ((id + 1) < LEN_BUF_CIRC_RATE ? (id++) : (id = 0))

// defined in main.c
extern uint8_t tic_tac;

bool
add_statistics_in_processes(process_t *processes,
                            const size_t tot_proc,
                            struct packet *pkt)
{
  static int last_tic;
  static uint8_t id_buff_circ;
  bool locate = false;

  if (last_tic != tic_tac)
    UPDATE_ID_BUFF(id_buff_circ);

  for (size_t i = 0; i < tot_proc; i++)
    {
      // caso o indice do buffer circular tenha atualizado,
      // pois ja deu o tempo pre definido, T_REFRESH,
      // apaga os dados antes de começar a escrever
      // para não incrementar em cima de valores antigos
      if (last_tic != tic_tac)
        {
          processes[i].net_stat.Bps_rx[id_buff_circ] = 0;
          processes[i].net_stat.Bps_tx[id_buff_circ] = 0;
          processes[i].net_stat.pps_rx[id_buff_circ] = 0;
          processes[i].net_stat.pps_tx[id_buff_circ] = 0;
          // printf("processes[%s]net_stat.Bps_rx[%d] - %d\n",
          // processes[i].name, id_buff_circ,
          // processes[i]net_stat.Bps_rx[ id_buff_circ]);
        }

      // caso o pacote<->processo ja tenha sido localizado e/ou não tenha
      // dados para atualizar
      // e o tempo para refresh não alterou,
      // podemos retornar pois não ha nada para atualizar
      if ( (locate || !pkt->lenght) && last_tic == tic_tac )
        return true;

      // processo<->pacote ja localizado ou sem dados para atualizar
      // e o tempo para refresh alterou,
      // apenas continua para zerar buffer dos demais processos
      // no tic_tac(segundo) atual
      if ( (locate || !pkt->lenght) && last_tic != tic_tac )
        continue;

      // percorre todas as conexões do processo...
      for (size_t j = 0; j < processes[i].total_conections; j++)
        {
          // printf ("local %d\n", processes[i].conection[j].local_port);
          // printf ("pkt   %d\n", pkt->local_port);

          // ... e verifica com o pacote recebido pela rede com base
          // na porta local, visto que somente um processo por vez pode usar
          // determinada porta
          if (processes[i].conection[j].local_port == pkt->local_port)
            {
              locate = true;

              if (pkt->direction == PKT_DOWN)
                {
                  processes[i].net_stat.pps_rx[id_buff_circ]++;
                  processes[i].net_stat.Bps_rx[id_buff_circ] += pkt->lenght;
                }
              else
                {
                  processes[i].net_stat.pps_tx[id_buff_circ]++;
                  processes[i].net_stat.Bps_tx[id_buff_circ] += pkt->lenght;
                }

              break;
            }
        }
    }

    // atualiza para id de buffer atual
    last_tic = tic_tac;
    return locate;
}
