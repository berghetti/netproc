
#include <string.h>
#include <errno.h>    // variable errno
#include "process.h"
#include "network.h"
#include "proc_rate.h"
#include "timer.h"
#include "statistics_proc.h"
#include "show.h"
#include "m_error.h"



// incremento circular de 0 até LEN_BUF_CIRC_RATE - 1
#define UPDATE_ID_BUFF(id) ((id + 1) < LEN_BUF_CIRC_RATE ? (id++) : (id = 0))

#define T_REFRESH 1.0   // intervalo de atualização do programa, não alterar


uint8_t id_buff_circ = 0;

process_t *processes = NULL;

int main(void)
{


  uint32_t tot_process_act = 0;
  tot_process_act = get_process_active_con(&processes, tot_process_act);


  struct sockaddr_ll link_level;
  memset(&link_level, 0, sizeof(link_level));


  if ( (create_socket()) == -1 )
    exit(EXIT_FAILURE);

  unsigned char *buffer = calloc(IP_MAXPACKET, 1);
  if (! buffer)
    fatal_error("Falha ao alocar buffer: %s", strerror(errno));

  struct packet packet = {0};
  // memset(&packet, 0, sizeof(packet));


 init_timer();
 ssize_t bytes = 0;
  while (1)
    {
      bytes = get_packets(&link_level, buffer, IP_MAXPACKET);

      if (bytes == -1){
        if(buffer)
          free(buffer);
        fatal_error("sniffer packets");
      }


      if (bytes > 0)
        {
          if (!parse_packet(&packet, buffer, &link_level))
            {
              goto PRINT;
            } // packet not is IP
        }


      packet.lenght = bytes;

      // print_packet(&packet);
      if (!add_statistics_in_processes(processes, tot_process_act, &packet))
        if (packet.lenght > 0)
          tot_process_act = get_process_active_con(&processes, tot_process_act);


      PRINT:
      if (timer() >= T_REFRESH)
        {
          calc_avg_rate(processes, tot_process_act);
          show_process(processes, tot_process_act);
          restart_timer();
          // atualiza o indice para gravar dados do
          // buffer circular a cada 1 segundo
          UPDATE_ID_BUFF(id_buff_circ);
        }

    }

  return 0;
}
