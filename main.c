
#include "headers-system.h"

#include "process.h"
#include "conection.h"
#include "network.h"
#include "proc_rate.h"


#define NSTOS 1000000000.0  // convert nanoseconds for seconds

// incremento circular de 0 até LEN_BUF_CIRC_RATE - 1
#define UPDATE_ID_BUFF(id) ((id + 1) < LEN_BUF_CIRC_RATE ? (id++) : (id = 0))

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

// void
// reset_counters_process(process_t *processes, const int total_process)
// {
//   for (int i = 0; i < total_process; i++)
//     {
//       // processes[i].net_stat.avg_pps_rx = 0;
//       // processes[i]net_stat.Bps_rx = 0;
//       // processes[i].net_stat.avg_pps_tx = 0;
//       // processes[i]net_stat.Bps_tx = 0;
//     }
// }

void
print_proc_net(process_t *processes, const size_t tot_process)
{
  cls();

  printf("%-5s\t %-45s %s\t %s\t %s\t %s \n",
        "PID", "PROGRAM", "PPS TX", "PPS RX", "UP", "DOWN");

  for (size_t i = 0; i < tot_process; i++)
    {
      printf("%-5d\t %-45s %d\t %d\t %d\t %-6d kbps\t \n",
            processes[i].pid,
            processes[i].name,
            processes[i].net_stat.avg_pps_tx,
            processes[i].net_stat.avg_pps_rx,
            processes[i].net_stat.avg_Bps_tx,
            processes[i].net_stat.avg_Bps_rx);
    }
}



bool
add_statistics_in_process(process_t *processes,
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
          // printf("processes[%s]net_stat.Bps_rx[%d] - %d\n", processes[i].name, id_buff_circ, processes[i]net_stat.Bps_rx[ id_buff_circ]);
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
 ssize_t bytes = 0;
  while (1)
    {
      bytes = get_packets(&link_level, buffer, IP_MAXPACKET);

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

      // print_packet(&packet);
      if (!add_statistics_in_process(processes, tot_process_act, &packet))
        if (packet.lenght > 0)
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
          UPDATE_ID_BUFF(id_buff_circ);
          // printf("%d\n", id_buff_circ);
        }

    }

  return 0;
}
