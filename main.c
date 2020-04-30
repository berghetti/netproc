
#include <string.h>
#include <errno.h>              // variable errno

#include "process.h"
#include "sock_sniff.h"
#include "network.h"
#include "proc_rate.h"
#include "timer.h"
#include "statistics_proc.h"
#include "sufix.h"
#include "clear.h"
#include "show.h"
#include "m_error.h"

#include <stdio.h>   //provisorio


// // incremento circular de 0 até LEN_BUF_CIRC_RATE - 1
// #define UPDATE_ID_BUFF(id) ((id + 1) < LEN_BUF_CIRC_RATE ? (id++) : (id = 0))

// a cada vez que o tempo de T_REFRESH é atingido
// esse valor é alterado (entre 0 e 1), para que outras partes
// do programa possam ter uma referencia de tempo
#define TIC_TAC(tic) ( (tic) ? (tic)-- : (tic)++ )

// intervalo de atualização do programa, não alterar
#define T_REFRESH 1.0

bool udp = true;
bool view_si = false;
bool view_bytes = false;

uint8_t tic_tac = 0;

process_t *processes = NULL;

int main(void)
{

  define_sufix();

  load_terminal();

  create_socket();

  uint32_t tot_process_act = 0;
  tot_process_act = get_process_active_con(&processes, tot_process_act);


  struct sockaddr_ll link_level = {0};


  unsigned char *buffer = calloc(IP_MAXPACKET, 1);
  if (! buffer)
    fatal_error(FATAL" Error alloc buffer packets: %s", strerror(errno));

  struct packet packet = {0};


 float m_timer = start_timer();
 ssize_t bytes = 0;
 // main loop
  while (1)
    {
      bytes = get_packets(&link_level, buffer, IP_MAXPACKET);

      if (bytes == -1){
        if(buffer)
          free(buffer);
        fatal_error("sniffer packets");
      }

      // se houver dados porem não foi possivel identificar o trafego,
      // não tem estatisticas para ser adicionada aos processos
      if (bytes > 0)
        if (!parse_packet(&packet, buffer, &link_level))
          goto PRINT;


      packet.lenght = bytes;

      // se não foi possivel identificar de qual processo o trafego pertence
      // é sinal que existe um novo processo que ainda não foi mapeado,
      // então atualizamos a lista de processos com conexões ativas
      if (!add_statistics_in_processes(processes, tot_process_act, &packet))
        if (bytes > 0)
          tot_process_act = get_process_active_con(&processes, tot_process_act);


      PRINT:
      if ( timer(m_timer) >= T_REFRESH )
        {
          calc_avg_rate(processes, tot_process_act);
          show_process(processes, tot_process_act);

          m_timer = restart_timer();
          // restart_timer();
          // atualiza o indice para gravar dados do
          // buffer circular a cada 1 segundo
          // printf("tic_tax %d\n", tic_tac);
          // UPDATE_ID_BUFF(id_buff_circ);
          // trocar update_id... por tic_tac
          TIC_TAC(tic_tac);
        }
    }

  return EXIT_SUCCESS;
}
