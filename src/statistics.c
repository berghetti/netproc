
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

#include <stdbool.h>
#include <net/if.h>
#include <netinet/in.h>   // IPPROTO_TCP, IPPROTO_UDP
#include <netinet/tcp.h>  // TCP_ESTABLISHED, TCP_TIME_WAIT...

#include "config.h"
#include "packet.h"
#include "processes.h"

// incremento circular de 0 até LEN_BUF_CIRC_RATE - 1
#define UPDATE_ID_BUFF( id ) ( ( id ) = ( ( id ) + 1 ) % LEN_BUF_CIRC_RATE )

static bool
conection_match_packet ( conection_t *restrict conection,
                         const struct packet *restrict pkt )
{
  if ( conection->protocol != pkt->protocol )
    return false;

  // TCP must correspond exactly with the packet, if not, skip
  if ( pkt->protocol == IPPROTO_TCP &&
       ( conection->local_port != pkt->local_port ||
         conection->remote_port != pkt->remote_port ||
         conection->remote_address != pkt->remote_address ) )
    {
      return false;
    }

  if ( pkt->protocol == IPPROTO_UDP )
    {
      // the local port is the only parameter that the kernel exports
      // in certain UDP situations (like a torrent),
      // if that is different, skip
      if ( conection->local_port != pkt->local_port )
        return false;

      // UDP connections with state TCP_ESTABLISHED must
      // correspond exactly with the packet, if not, skip
      else if ( conection->state == TCP_ESTABLISHED &&
                ( conection->remote_port != pkt->remote_port ||
                  conection->remote_address != pkt->remote_address ) )
        return false;

      // udp connections have no state, however the kernel tries
      // to correlate udp packets into connections, when it is able
      // to assign the virtual state TCP_ESTABLISHED, when it is
      // unable to assign the state TCP_CLOSE, and it ends up not
      // exporting some data in /proc/net/udp, but we can "retrieve"
      // that information from the captured packet
      else if ( conection->state == TCP_CLOSE )
        {
          conection->local_address = pkt->local_address;
          conection->remote_address = pkt->remote_address;
          conection->remote_port = pkt->remote_port;
        }
    }

  return true;
}

bool
add_statistics_in_processes ( struct processes *processes,
                              const struct packet *pkt,
                              const struct config_op *co )
{
  static int last_tic;
  static uint8_t id_buff_circ;

  // time geral do programa atualizou, desloca o indice do buffer
  if ( last_tic != co->tic_tac )
    UPDATE_ID_BUFF ( id_buff_circ );

  bool locate = false;
  size_t c;
  for ( process_t **proc = processes->proc; *proc; proc++ )
    {
      process_t *process = *proc;
      // caso o indice do buffer circular tenha atualizado,
      // pois ja deu o tempo pre definido, T_REFRESH,
      // apaga os dados antes de começar a escrever
      // para não incrementar em cima de valores antigos
      if ( last_tic != co->tic_tac )
        {
          process->net_stat.Bps_rx[id_buff_circ] = 0;
          process->net_stat.Bps_tx[id_buff_circ] = 0;
          process->net_stat.pps_rx[id_buff_circ] = 0;
          process->net_stat.pps_tx[id_buff_circ] = 0;

          process->net_stat.bytes_last_sec_rx = 0;
          process->net_stat.bytes_last_sec_tx = 0;

          // zera estatisticas da conexões tambem
          if ( co->view_conections )
            {
              for ( c = 0; c < process->total_conections; c++ )
                {
                  process->conection[c].net_stat.Bps_rx[id_buff_circ] = 0;
                  process->conection[c].net_stat.Bps_tx[id_buff_circ] = 0;
                  process->conection[c].net_stat.pps_rx[id_buff_circ] = 0;
                  process->conection[c].net_stat.pps_tx[id_buff_circ] = 0;
                }
            }
        }

      // caso o pacote<->processo ja tenha sido localizado e/ou não tenha
      // dados para atualizar
      // e o tempo para refresh não alterou,
      // podemos retornar pois não ha nada para atualizar
      if ( ( locate || !pkt->lenght ) && last_tic == co->tic_tac )
        return true;

      // processo<->pacote ja localizado ou sem dados para atualizar
      // e o tempo para refresh alterou,
      // apenas continua para zerar buffer dos demais processos
      // no co->tic_tac(segundo) atual
      if ( ( locate || !pkt->lenght ) && last_tic != co->tic_tac )
        continue;

      // percorre todas as conexões do processo...
      for ( c = 0; c < process->total_conections; c++ )
        {
          // check if packet math con conection from this process
          if ( !conection_match_packet ( &process->conection[c], pkt ) )
            continue;

          // pass all test
          locate = true;

          process->conection[c].if_index = pkt->if_index;

          if ( pkt->direction == PKT_DOWN )
            {  // estatisticas geral do processo
              process->net_stat.pps_rx[id_buff_circ]++;
              process->net_stat.Bps_rx[id_buff_circ] += pkt->lenght;

              process->net_stat.bytes_last_sec_rx += pkt->lenght;

              process->net_stat.tot_Bps_rx += pkt->lenght;

              // adicionado estatisticas exclusiva da conexão
              if ( co->view_conections )
                {
                  process->conection[c].net_stat.pps_rx[id_buff_circ]++;
                  process->conection[c].net_stat.Bps_rx[id_buff_circ] +=
                          pkt->lenght;
                }
            }
          else
            {  // estatisticas geral do processo
              process->net_stat.pps_tx[id_buff_circ]++;
              process->net_stat.Bps_tx[id_buff_circ] += pkt->lenght;

              process->net_stat.bytes_last_sec_tx += pkt->lenght;

              process->net_stat.tot_Bps_tx += pkt->lenght;

              // adicionado estatisticas exclusiva da conexão
              if ( co->view_conections )
                {
                  process->conection[c].net_stat.pps_tx[id_buff_circ]++;
                  process->conection[c].net_stat.Bps_tx[id_buff_circ] +=
                          pkt->lenght;
                }
            }

          break;
        }
    }

  // atualiza para id de buffer atual
  last_tic = co->tic_tac;
  return locate;
}
