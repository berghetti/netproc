
#include "statistics_proc.h"


bool
add_statistics_in_processes(process_t *processes,
                          const size_t tot_proc,
                          struct packet *pkt)
{
  static uint8_t last = 0;
  bool locate = false;

  for (size_t i = 0; i < tot_proc; i++)
    {
      // caso o indice do buffer circular tenha atualizado,
      // pois ja deu o tempo pre definido, T_REFRESH,
      // apaga os dados antes de começar a escrever
      // para não sobrescrever em cima de valores antigos
      if (last != id_buff_circ)
        {
          processes[i].net_stat.Bps_rx[id_buff_circ] = 0;
          processes[i].net_stat.Bps_tx[id_buff_circ] = 0;
          processes[i].net_stat.pps_rx[id_buff_circ] = 0;
          processes[i].net_stat.pps_tx[id_buff_circ] = 0;
          // printf("processes[%s]net_stat.Bps_rx[%d] - %d\n",
          // processes[i].name, id_buff_circ,
          // processes[i]net_stat.Bps_rx[ id_buff_circ]);
        }

      // caso o pacote/processo ja tenha sido localizado
      // e o tempo para refresh não alterou
      if (locate && last == id_buff_circ)
        break;

      // processo/pacote ja localizado ou sem dados para atualizar,
      // apenas continua para zerar buffer dos demais processos
      // que ficarem sem receber dados por T_REFRESH segundo
      if (locate || !pkt->lenght)
        continue;


      for (size_t j = 0; j < processes[i].total_conections; j++)
        {
          // printf ("local %d\n", processes[i].conection[j].local_port);
          // printf ("pkt   %d\n", pkt->local_port);

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
    last = id_buff_circ;
    return locate;
}
