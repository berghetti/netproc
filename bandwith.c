
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>


#define NSTOS 1000000000.0  // convert nanoseconds for seconds


float diff(struct timespec *init, struct timespec *end){
  float a;
  a = end->tv_sec - init->tv_sec;
  a += (end->tv_nsec - init->tv_nsec) / NSTOS;
  return a;
}

void cls(void){
   printf("\033[2J");   // Limpa a tela
   printf("\033[0;0H"); // Devolve o cursor para a linha 0, coluna 0

   //https://pt.stackoverflow.com/questions/58453/como-fazer-efeito-de-loading-no-terminal-em-apenas-uma-linha
}


int main(int argc, char **argv) {

  if (argc != 2){
    printf("usage: %s interface\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  unsigned char *buffer = (unsigned char *)malloc(IP_MAXPACKET); /* maximum packet size */
  struct timespec initTime, endTime;
  struct sockaddr_ll pkt, my_sock;
  socklen_t size_sockaddr = sizeof(pkt);
  ssize_t bytes_received = 0;

  int packets_ingoing = 0, packets_outgoing = 0;
  int total_packets_ingoing = 0, total_packets_outgoing = 0;

  int bytes_ingoing = 0, bytes_outgoing = 0;
  int total_bytes_ingoing = 0, total_bytes_outgoing = 0;

  int packets_aggregate = 0, bytes_aggregate = 0;
  int total_packets_aggregate = 0, total_bytes_aggregate = 0;

  int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if(sock == -1){
    //necessary root
    perror("Necessario root");
    exit(EXIT_FAILURE);
  }

  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 100000; // 1/10 of second

  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout)) == -1){
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  // ouvir somente da interface especificada
  char *iface = argv[1];
  int iface_index = if_nametoindex(iface);
  if(!iface_index){
    perror("Interface");
    exit(EXIT_FAILURE);
  }
  memset(&my_sock, 0, sizeof(my_sock));
  my_sock.sll_family = PF_PACKET;
  my_sock.sll_protocol = htons(ETH_P_ALL);
  my_sock.sll_ifindex = iface_index;
  if (bind(sock, (struct sockaddr *)&my_sock, sizeof(my_sock)) == -1){
      perror("bind");
      exit(EXIT_FAILURE);
  }


  if (clock_gettime(CLOCK_MONOTONIC, &initTime) == -1){
    perror("clock");
    exit(EXIT_FAILURE);
  }

  while(1) {

    bytes_received = recvfrom(sock , buffer , IP_MAXPACKET , 0, (struct sockaddr *) &pkt, &size_sockaddr);

    if (bytes_received > 0 && bytes_received != -1){
      if(pkt.sll_pkttype == PACKET_OUTGOING){
        // upload
        packets_outgoing++;
        total_packets_outgoing++;

        bytes_outgoing += bytes_received;
        total_bytes_outgoing += bytes_received; // total upload

      }
      else{
        // download
        packets_ingoing++;
        total_packets_ingoing++;

        bytes_ingoing += bytes_received;
        total_bytes_ingoing += bytes_received; // total download
      }

      packets_aggregate++;
      total_packets_aggregate++;

      bytes_aggregate += bytes_received;
      total_bytes_aggregate += bytes_received;
    }


    if (clock_gettime(CLOCK_MONOTONIC, &endTime) == -1 )
      printf("erro");

    if(diff(&initTime, &endTime) >= 1.0){

      cls();
      printf("------------------------------------------------------------------------\n"
             " T. bytes  | T. pkts   | bytes in  | pkts in   | bytes out | pkts out  |\n"
             "------------------------------------------------------------------------\n"
             "%-11d|%-11d|%-11d|%-11d|%-11d|%-11d\n",
              total_bytes_aggregate, total_packets_aggregate,
              total_bytes_ingoing, total_packets_ingoing,
              total_bytes_outgoing, total_packets_outgoing);

      printf("\n\n");
      printf("------------------------------------------------------------------------\n"
             "Download | %d Kibps\n"
             "           %d pps\n"
             "\n"
             "Upload   | %d Kibps\n"
             "           %d pps\n",
             (bytes_ingoing * 8) / 1024,
             packets_ingoing,
             (bytes_outgoing * 8) / 1024,
             packets_outgoing);


      initTime = endTime;

      packets_aggregate = 0;
      bytes_aggregate = 0;
      packets_ingoing = 0;
      packets_outgoing = 0;
      bytes_ingoing = 0;
      bytes_outgoing = 0;
    }

    // pegar dados cabeçalho ethernet
    // struct ethhdr *l2;
    // l2 = (struct ethhdr *) buffer;
    //
    //
    // printf("MAC DST: %02x:%02x:%02x:%02x:%02x:%02x\n",
    //       l2->h_dest[0],
    //       l2->h_dest[1],
    //       l2->h_dest[2],
    //       l2->h_dest[3],
    //       l2->h_dest[4],
    //       l2->h_dest[5]);
    // printf("MAC SRC: %02x:%02x:%02x:%02x:%02x:%02x\n",
    //       l2->h_source[0],
    //       l2->h_source[1],
    //       l2->h_source[2],
    //       l2->h_source[3],
    //       l2->h_source[4],
    //       l2->h_source[5]);

    //
    // printf("Protocol: 0x%04x\n\n", ntohs(l2->h_proto));

    // printf("pai: total_packets: %d\n", total_packets);

    // if (clock_gettime(CLOCK_MONOTONIC, &initTime) == -1)
    //   printf("erro");

    // if (clock_gettime(CLOCK_MONOTONIC, &endTime) == -1)
    //   printf("erro");

    // time = endTime.tv_sec - initTime.tv_sec;
    // time += (endTime.tv_nsec - initTime.tv_nsec) / NSTOS;
    //
    // pps = total_packets / time;

    // printf("\rtime: %f", time);
    //printf("\rpacket per second: %f", pps);

    //
    //
    // struct iphdr *ip_packet = (struct iphdr *)buffer;
    // memset(&source_adress, 0, sizeof(source_adress));
    // source_adress.s_addr = ip_packet->saddr;
    // memset(&dest_adress, 0, sizeof(dest_adress));
    // dest_adress.s_addr = ip_packet->daddr;
    //
    // printf("Incoming Packet: \n");
    // printf("Packet Size (bytes): %d\n",ntohs(ip_packet->tot_len));
    // printf("Source Address: %s\n", (char *)inet_ntoa(source_adress));
    // printf("Destination Address: %s\n", (char *)inet_ntoa(dest_adress));
    // printf("Identification: %d\n\n", ntohs(ip_packet->id));
  }

  return 0;
}
