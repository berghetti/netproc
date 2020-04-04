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

  uint8_t div_rx;
  uint8_t div_tx;

  for (size_t i = 0; i < tot_proc; i++)
    {
      sum_rx = 0;
      sum_tx = 0;
      div_rx = 0;
      div_tx = 0;

      for (size_t j = 0; j < LEN_BUF_CIRC_RATE; j++)
        {
          // if (proc[i].Bps_rx[j])
            // {
              printf("buffer rx[%d] - %d\n", j, proc[i].Bps_rx[j]);
              sum_rx += proc[i].Bps_rx[j];
              // proc[i].Bps_rx[j] = 0; // zera o valor ja lido para que nao acumule com dados ja lidos
              div_rx++;
            // }

          // if (proc[i].Bps_tx[j])
            // {
              sum_tx += proc[i].Bps_tx[j];
              div_tx++;
            // }
        }
        printf("sum_rx %d\n", sum_rx);
        proc[i].avg_rx = (sum_rx) ? ((sum_rx) / 1024) / LEN_BUF_CIRC_RATE : 0;
        proc[i].avg_tx = (sum_tx) ? (sum_tx * 8) / LEN_BUF_CIRC_RATE : 0;
        // getchar();
    }


}

uint8_t last = 0;
int
add_statistics_in_process(process_t *processes,
                          const size_t tot_proc,
                          struct packet *pkt)
{

  // last = (id_buff_circ >= 1) ? id_buff_circ - 1 : LEN_BUF_CIRC_RATE - 1;

  for (size_t i = 0; i < tot_proc; i++)
    {
      // puts("add statistics");
      // printf("total de conexoes do processo %s - %d\n",
      //  processes[i].name,
      //  processes[i].total_conections);

      for (size_t j = 0; j < processes[i].total_conections; j++)
        {
          // printf ("local %d\n", processes[i].conection[j].local_port);
          // printf ("pkt   %d\n", pkt->local_port);

          if (processes[i].conection[j].local_port == pkt->local_port)
            {

              if (pkt->direction == PKT_DOWN)
                {
                  if (processes[i].n_elements_buf_rx >= LEN_BUF_CIRC_RATE)
                    processes[i].n_elements_buf_rx = 0;

                  // printf("buffer rx[%d] - recebe %d\n",
                  // processes[i].n_elements_buf_rx, pkt->lenght);

                  // printf("pkt len RX st - %d\n", pkt->lenght);



                  // printf("last - %d\n", last);

                  if (last != id_buff_circ)
                    {
                      puts("zerando para escrever");
                      processes[i].Bps_rx[ id_buff_circ ] = 0;
                      last = id_buff_circ;
                    }

                  processes[i].pps_rx++;
                  processes[i].Bps_rx[ id_buff_circ ] += pkt->lenght;

                  // last = id_buff_circ;

                  // printf("buffer rx[%d] - %d\n", id_buff_circ,
                  // processes[i].Bps_rx[id_buff_circ]);
                  // getchar();
                }
              else
                {
                  // printf("pkt len TX st - %d\n", pkt->lenght);
                  if (processes[i].n_elements_buf_tx >= LEN_BUF_CIRC_RATE)
                    processes[i].n_elements_buf_tx = 0;

                  processes[i].pps_tx++;
                  processes[i].Bps_tx[ id_buff_circ ] += pkt->lenght;

                  // printf("buffer tx[%d] - %d\n", id_buff_circ,
                  // processes[i].Bps_rx[id_buff_circ]);
                }

              return 1;
            }
          else
            {
              // if (processes[i].n_elements_buf_rx >= LEN_BUF_CIRC_RATE)
              //   processes[i].n_elements_buf_rx = 0;
              //
              // if (processes[i].n_elements_buf_tx >= LEN_BUF_CIRC_RATE)
              //   processes[i].n_elements_buf_tx = 0;

              // processes[i].Bps_rx[ processes[i].n_elements_buf_rx++ ] = 0;
              // processes[i].Bps_tx[ processes[i].n_elements_buf_tx++ ] = 0;
            }

        }

    }
    // não localizado o processo que fez essa conexao
    return 0;
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


      if ( ! parse_packet(&packet, buffer, &link_level))
      {
        // puts("parse deu ruim");
        goto PRINT;
      } // packet not is IP

        // continue;


      packet.lenght = bytes;
      // printf("pkt len antes %d\n", packet.lenght);

      // print_packet(&packet);
      if (!add_statistics_in_process(processes, tot_process_act, &packet))
        // lembrar que aqui esta perdendo estatisticas de rede, checar...
        tot_process_act = get_process_active_con(&processes, tot_process_act);



      PRINT:
      if (clock_gettime(CLOCK_MONOTONIC, &endTime) == -1 )
        perror("clock_gettime");


      if (diff(&initTime, &endTime) >= 1.0)
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
