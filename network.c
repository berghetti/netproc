#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "network.h"


int create_socket()
{

  // int sock;
  if ( (sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1 )
  {
    perror("Necessario root");
    return -1;
  }


  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 100000; // 1/10 of second
  // set timeout for read in socket
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout)) == -1)
    {
      perror("setsockopt");
      return -1;
    }

  struct sockaddr_ll my_sock;
  memset(&my_sock, 0 , sizeof(my_sock));
  my_sock.sll_family = AF_PACKET;
  my_sock.sll_protocol = htons(ETH_P_ALL);
  my_sock.sll_ifindex = 0; // 0 equal all interfaces sniffer


  if (bind(sock, (struct sockaddr *)&my_sock, (sizeof(my_sock))) == -1){
      perror("bind");
      return -1;
  }

  return sock;

}


int get_packets(struct sockaddr_ll *link_level, unsigned char *buffer, const int lenght)
{
  socklen_t link_level_size = sizeof(struct sockaddr_ll);


  ssize_t bytes_received = recvfrom(sock , buffer , lenght , 0,
                          (struct sockaddr *) link_level, &link_level_size);

  if (bytes_received >= 0 && bytes_received != -1)
    return bytes_received;

  if (bytes_received == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
    return 0;

  if(bytes_received == -1)
    perror("recvfrom");

  return -1;

}


void print_l2(struct ethhdr *l2, const int flag)
{
  if (flag == SRC || flag == BOTH)
    {
      printf("MAC SRC: %02x:%02x:%02x:%02x:%02x:%02x\n",
            l2->h_source[0],
            l2->h_source[1],
            l2->h_source[2],
            l2->h_source[3],
            l2->h_source[4],
            l2->h_source[5]);
    }
  if (flag == DST || flag == BOTH)
    {
      printf("MAC DST: %02x:%02x:%02x:%02x:%02x:%02x\n",
            l2->h_dest[0],
            l2->h_dest[1],
            l2->h_dest[2],
            l2->h_dest[3],
            l2->h_dest[4],
            l2->h_dest[5]);
    }
}
