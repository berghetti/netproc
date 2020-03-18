#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>

#include "process.h"
#include "network.h"

#define PKT_DOWNLOAD 1
#define PKT_UPLOAD   2


struct packet{
  uint32_t local_address;
  uint16_t local_port;
  uint16_t lenght;
  uint8_t  direction;
};


// int add_statistics_in_process(process_t processes, const int tot_proc, struct sockaddr_ll *packet, const int bytes)
// {
//
//   for (size_t i = 0; i < tot_proc; i++)
//     {
//       if (processes->conection.local_port == packet->lport)
//         {
//           if (packet->direction == PKT_DOWNLOAD)
//             processes->conection.rx++;
//           else
//             processes->conetion.tx++;
//
//           return 1;
//         }
//     }
//
//     return -1; // processo não localizado para essas caracteristicas
// }

int parse_packet(struct sockaddr_ll *ll, unsigned char *buf)
{
  struct ethhdr *l2;
  struct iphdr  *l3;
  struct tcphdr *l4;

  l2 = (struct ethhdr *) buf;
  if(ntohs(l2->h_proto) != ETH_P_IP) // not is a packet internet protocol
    return 0;

  // criar packet

  return 1;

}



 int sock;

int main(int argc, char **argv)
{

  process_t *processes = NULL;
  int tot_process_act = 0;
  tot_process_act = get_process_active_con(&processes);


  struct sockaddr_ll link_level;
  memset(&link_level, 0, sizeof(link_level));


  if ( (create_socket()) == -1 )
    exit(EXIT_FAILURE);

  unsigned char *buffer = malloc(IP_MAXPACKET);
  struct ethhdr *l2;
  struct packet packet;
  while (1)
    {

      int bytes = get_packets(&link_level, buffer, IP_MAXPACKET);

      if (bytes == -1){
        free(buffer);
        exit(EXIT_FAILURE);
      }


      if ( bytes > 0)
        {
          // l2 = (struct ethhdr *) buffer;
          // if(ntohs(l2->h_proto) != ETH_P_IP) // not is a packet internet protocol
          //   continue;

          if ( !parse_packet(&link_level, buffer))
            puts("pacote invalido");
          else
            puts("pacote bom");

          // add_statistics_in_process(processes, tot_process_act, &link_level, bytes);

          // if(link_level.sll_pkttype == PACKET_OUTGOING )
          //   { // upload
          //     print_l2(l2, BOTH);
          //   }
          // else
          //   { // download
          //
          //   }

        }
    }



  // struct packet *packets = NULL;
  // while(1)
  //   {
  //     if (get_packets(&packets) == -1)
  //       exit(EXIT_FAILURE);
  //
  //     if (add_statistics_in_process(processes, tot_process, packets) == -1)
  //         tot_process_act = get_process_active_con(&processes); // atualiza lista de processos
  //
  //
  //     if (diff() >= 1.0) // ja deu 1 segundo?
  //       printf_process();
  //
  //   }
  //
  //
  // print_process(processes, tot_process_act);
  //
  // free_process(process, tot_process_act);

  return 0;
}
