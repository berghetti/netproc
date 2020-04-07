#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>

#include <time.h>

#include "process.h"
#include "network.h"


#define NSTOS 1000000000.0  // convert nanoseconds for seconds

#define T_REFRESH 1.0       // intervalo de atualização

uint8_t id_buff_circ = 0;

float diff(struct timespec *init, struct timespec *end)
{
  float dif;
  dif = end->tv_sec - init->tv_sec;
  dif += (end->tv_nsec - init->tv_nsec) / NSTOS;
  return dif;
}

void cls(void){
   printf("\033[2J");   // Limpa a tela
   printf("\033[0;0H"); // Devolve o cursor para a linha 0, coluna 0

   //https://pt.stackoverflow.com/questions/58453/como-fazer-efeito-de-loading-no-terminal-em-apenas-uma-linha
}

void
reset_counters_process(process_t *processes, const int total_process)
{
  // for (int i = 0; i < total_process; i++)
  //   {
  //     // processes[i].pps_rx = 0;
  //     // processes[i].Bps_rx = 0;
  //     // processes[i].pps_tx = 0;
  //     // processes[i].Bps_tx = 0;
  //   }
}

void
print_proc_net(process_t *processes, const size_t tot_process)
{
  cls();

  printf("%-5s\t %-45s %s\t %s\t %s\t %s \n",
        "PID", "PROGRAM", "PPS TX", "PPS RX", "UP", "DOWN");

  for (size_t i = 0; i < tot_process; i++)
    {
      printf("%-5d\t %-45s %d\t %d\t %d\t %-6d kbps\t \n",
      // printf("%-5d\t \n",

            processes[i].pid,
            processes[i].name,
            processes[i].pps_tx,
            processes[i].pps_rx,
            processes[i].avg_tx,
            processes[i].avg_rx
            // processes[i].avg_rx
            // (processes[i].Bps_tx) ? (processes[i].Bps_tx * 8) / 1024 : 0,
            // (processes[i].Bps_rx) ? (processes[i].Bps_rx * 8) / 1024 : 0
          );
    }
}

void
calc_avg_rate(process_t *proc, const size_t tot_proc )
{
  size_t sum_rx;
  size_t sum_tx;

  for (size_t i = 0; i < tot_proc; i++)
    {
      sum_rx = 0;
      sum_tx = 0;

      for (size_t j = 0; j < LEN_BUF_CIRC_RATE; j++)
        {
          // printf("pid %d - buffer rx[%d] - %d\n", proc[i].pid, j, proc[i].Bps_rx[j]);
          sum_rx += proc[i].Bps_rx[j];
          sum_tx += proc[i].Bps_tx[j];

        }
        // printf("sum_rx %d\n", sum_rx);
        // putchar('\n');
        proc[i].avg_rx = (sum_rx) ? ((sum_rx) / 1024) / LEN_BUF_CIRC_RATE : 0;
        proc[i].avg_tx = (sum_tx) ? ((sum_tx) / 1024) / LEN_BUF_CIRC_RATE : 0;

    }
}


uint8_t last = 0;
bool
add_statistics_in_process(process_t *processes,
                          const size_t tot_proc,
                          struct packet *pkt)
{

  bool locate = false;

  for (size_t i = 0; i < tot_proc; i++)
    {
      // caso o indice do buffer circular tenha atualizado,
      // pois ja deu o tempo pre definido, T_REFRESH,
      // apaga os dados antes de começar a escrever
      // para não sobrescrever em cima de valores antigos
      if (last != id_buff_circ)
        {
          processes[i].Bps_rx[ id_buff_circ] = 0;
          processes[i].Bps_tx[ id_buff_circ] = 0;
          // processes[i].pps_rx = 0;
          // printf("processes[%s].Bps_rx[%d] - %d\n", processes[i].name, id_buff_circ, processes[i].Bps_rx[ id_buff_circ]);
        }

      // processo ja localizado, apenas continua
      // para zerar buffer dos demais processos
      // que por ficaram sem receber dados por
      // T_REFRESH segundo
      if (locate)
        continue;

      // caso n
      if (!pkt->lenght)
        {
          // puts("pulando 0 bytes");
          continue;
        }

      for (size_t j = 0; j < processes[i].total_conections; j++)
        {
          // printf ("local %d\n", processes[i].conection[j].local_port);
          // printf ("pkt   %d\n", pkt->local_port);

          if (processes[i].conection[j].local_port == pkt->local_port)
            {
              locate = true;

              if (pkt->direction == PKT_DOWN)
                {
                  processes[i].pps_rx++;
                  processes[i].Bps_rx[id_buff_circ] += pkt->lenght;

                  // printf("buffer rx[%d] - %d\n", id_buff_circ,
                  // processes[i].Bps_rx[id_buff_circ]);
                }
              else
                {
                  processes[i].pps_tx++;
                  processes[i].Bps_tx[id_buff_circ] += pkt->lenght;

                  // printf("buffer tx[%d] - %d\n", id_buff_circ,
                  // processes[i].Bps_rx[id_buff_circ]);
                }

              break;

            }
        }
    }

    // atualiza para id de buffer atual
    last = id_buff_circ;
    return locate;
}



int main(void)
{

  process_t *processes = NULL;
  uint32_t tot_process_act = 0;
  tot_process_act = get_process_active_con(&processes, tot_process_act);


  struct sockaddr_ll link_level;
  memset(&link_level, 0, sizeof(link_level));


  if ( (create_socket()) == -1 )
    exit(EXIT_FAILURE);

  unsigned char *buffer = calloc(IP_MAXPACKET, 1);
  if (! buffer)
    {
      perror("calloc");
      exit(EXIT_FAILURE);
    }

  struct packet packet;
  memset(&packet, 0, sizeof(packet));
  struct timespec initTime, endTime;


  if (clock_gettime(CLOCK_MONOTONIC, &initTime) == -1)
    {
      perror("clock_gettime");
      exit(EXIT_FAILURE);
    }

  while (1)
    {
      ssize_t bytes = get_packets(&link_level, buffer, IP_MAXPACKET);

      if (bytes == -1){
        if(buffer)
          free(buffer);
        perror("get_packets");
        exit(EXIT_FAILURE);
      }
      // printf("bytes len %d\n", bytes);

      // if (bytes == 0)
      //   goto PRINT;

      if (bytes > 0)
        {
          if (!parse_packet(&packet, buffer, &link_level))
            {
              goto PRINT;
            } // packet not is IP
        }


      packet.lenght = bytes;
      // printf("pkt len antes %d\n", packet.lenght);

      // print_packet(&packet);
      if (!add_statistics_in_process(processes, tot_process_act, &packet))
        // lembrar que aqui esta perdendo estatisticas de rede, checar...
        tot_process_act = get_process_active_con(&processes, tot_process_act);



      PRINT:
      if (clock_gettime(CLOCK_MONOTONIC, &endTime) == -1 )
        perror("clock_gettime");


      if (diff(&initTime, &endTime) >= T_REFRESH)
        {
          calc_avg_rate(processes, tot_process_act);
          print_proc_net(processes, tot_process_act);
          // reset_counters_process(processes, tot_process_act);
          initTime = endTime;


          // atualiza o indice para gravar dados do buffer circular a cada 1 segundo
          id_buff_circ = ((id_buff_circ + 1) < LEN_BUF_CIRC_RATE) ? id_buff_circ + 1 : 0;
        }


    }

  return 0;
}
