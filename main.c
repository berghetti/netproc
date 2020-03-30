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
  for (int i = 0; i < total_process; i++)
    {
      processes[i].pps_rx = 0;
      processes[i].Bps_rx = 0;
      processes[i].pps_tx = 0;
      processes[i].Bps_tx = 0;
    }
}

void
print_proc_net(process_t *processes, const int tot_process)
{
  cls();

  printf("%-5s\t %-45s %s\t %s\t %s\t %s \n",
        "PID", "PROGRAM", "PPS TX", "PPS RX", "UP", "DOWN");

  for (int i = 0; i < tot_process; i++)
    {
      printf("%-5d\t %-45s %d\t %d\t %d\t %-6d kbps\t \n",
            processes[i].pid,
            processes[i].name,
            processes[i].pps_tx,
            processes[i].pps_rx,
            (processes[i].Bps_tx) ? (processes[i].Bps_tx * 8) / 1024 : 0,
            (processes[i].Bps_rx) ? (processes[i].Bps_rx * 8) / 1024 : 0
          );
    }
}


int
add_statistics_in_process(process_t *processes,
                          const uint32_t tot_proc,
                          struct packet *pkt)
{

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
                  processes[i].pps_rx++;
                  processes[i].Bps_rx += pkt->lenght;
                }
              else
                {
                  processes[i].pps_tx++;
                  processes[i].Bps_tx += pkt->lenght;
                }

              return 1;
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

      if (bytes == 0)
        goto PRINT;


      if ( !parse_packet(&link_level, buffer, &packet) ) // packet not is IP
        goto PRINT;
        // continue;


      packet.lenght = bytes;

      // print_packet(&packet);
      if ( ! add_statistics_in_process(processes, tot_process_act, &packet) )
        {
          // puts("BUSCANDO PROCESSOS/CONEXOES...");
            // puts("buscando novos processos");
            // checar o free, perdendo estatisticas ao checar novos processos/conexoes
            // no tempo em que ja temos estatisticas e nao deu tempo de atualizar/printar
            // porem buscamos novas processos, perdemos essas estatisticas

            // free_process(processes, tot_process_act);
            tot_process_act = get_process_active_con(&processes, tot_process_act);

            // tot_process_act = refresh_process_active_con(&processes, tot_process_act);
            // continue;
        }

      PRINT:
      if (clock_gettime(CLOCK_MONOTONIC, &endTime) == -1 )
        perror("clock_gettime");


      if (diff(&initTime, &endTime) >= 1.0)
        {
          print_proc_net(processes, tot_process_act);
          reset_counters_process(processes, tot_process_act);
          initTime = endTime;
        }


    }

  return 0;
}
