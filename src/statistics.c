
/*
 *  Copyright (C) 2020 Mayco S. Berghetti
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdio.h>  // provisório

#include "packet.h"
#include "process.h"

// incremento circular de 0 até LEN_BUF_CIRC_RATE - 1
#define UPDATE_ID_BUFF( id ) ( ( id ) = ( ( id ) + 1 ) % LEN_BUF_CIRC_RATE )
// ( ( id + 1 ) < LEN_BUF_CIRC_RATE ? ( id++ ) : ( id = 0 ) )

// defined in main.c
extern uint8_t tic_tac;
extern bool view_conections;

bool
add_statistics_in_processes ( process_t *restrict processes,
                              const size_t tot_proc,
                              const struct packet *restrict pkt )
{
  static int last_tic;
  static uint8_t id_buff_circ;
  bool locate = false;

  if ( last_tic != tic_tac )
    UPDATE_ID_BUFF ( id_buff_circ );

  for ( size_t i = 0; i < tot_proc; i++ )
    {
      // caso o indice do buffer circular tenha atualizado,
      // pois ja deu o tempo pre definido, T_REFRESH,
      // apaga os dados antes de começar a escrever
      // para não incrementar em cima de valores antigos
      if ( last_tic != tic_tac )
        {
          processes[i].net_stat.Bps_rx[id_buff_circ] = 0;
          processes[i].net_stat.Bps_tx[id_buff_circ] = 0;
          processes[i].net_stat.pps_rx[id_buff_circ] = 0;
          processes[i].net_stat.pps_tx[id_buff_circ] = 0;

          // zera estatisticas da conexões tambem
          if ( view_conections )
            {
              for ( size_t c = 0; c < processes[i].total_conections; c++ )
                {
                  processes[i].conection[c].net_stat.Bps_rx[id_buff_circ] = 0;
                  processes[i].conection[c].net_stat.Bps_tx[id_buff_circ] = 0;
                  processes[i].conection[c].net_stat.pps_rx[id_buff_circ] = 0;
                  processes[i].conection[c].net_stat.pps_tx[id_buff_circ] = 0;
                }
            }
        }

      // caso o pacote<->processo ja tenha sido localizado e/ou não tenha
      // dados para atualizar
      // e o tempo para refresh não alterou,
      // podemos retornar pois não ha nada para atualizar
      if ( ( locate || !pkt->lenght ) && last_tic == tic_tac )
        return true;

      // processo<->pacote ja localizado ou sem dados para atualizar
      // e o tempo para refresh alterou,
      // apenas continua para zerar buffer dos demais processos
      // no tic_tac(segundo) atual
      if ( ( locate || !pkt->lenght ) && last_tic != tic_tac )
        continue;

      // percorre todas as conexões do processo...
      for ( size_t j = 0; j < processes[i].total_conections; j++ )
        {
          // fprintf(stderr, "process - %d\nkt - %d\n",
          // processes[i].conection[j].remote_port, pkt->remote_port);
          // ... e verifica com o pacote recebido pela rede com base
          // na porta local, visto que somente um processo por vez pode usar
          // determinada porta
          if ( processes[i].conection[j].local_port == pkt->local_port )
            {
              locate = true;

              if ( pkt->direction == PKT_DOWN )
                {  // estatisticas geral do processo
                  processes[i].net_stat.pps_rx[id_buff_circ]++;
                  processes[i].net_stat.Bps_rx[id_buff_circ] += pkt->lenght;

                  processes[i].net_stat.tot_Bps_rx += pkt->lenght;

                  // adicionado estatisticas exclusica da conexão
                  if ( view_conections )
                    {
                      processes[i].conection[j].net_stat.pps_rx[id_buff_circ]++;
                      processes[i].conection[j].net_stat.Bps_rx[id_buff_circ] +=
                              pkt->lenght;
                    }
                }
              else
                {  // estatisticas geral do processo
                  processes[i].net_stat.pps_tx[id_buff_circ]++;
                  processes[i].net_stat.Bps_tx[id_buff_circ] += pkt->lenght;

                  processes[i].net_stat.tot_Bps_tx += pkt->lenght;

                  // adicionado estatisticas exclusica da conexão
                  if ( view_conections )
                    {
                      processes[i].conection[j].net_stat.pps_tx[id_buff_circ]++;
                      processes[i].conection[j].net_stat.Bps_tx[id_buff_circ] +=
                              pkt->lenght;
                    }
                }

              break;
            }
        }
    }

  // atualiza para id de buffer atual
  last_tic = tic_tac;
  return locate;
}
