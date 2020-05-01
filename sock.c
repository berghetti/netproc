
#include <sys/types.h>          // socket
#include <sys/socket.h>         // socket
#include <arpa/inet.h>          // htons
#include <string.h>             // strerror
#include <errno.h>              // variable errno
#include <net/if.h>             // if_nametoindex
#include <linux/if_packet.h>    // struct sockaddr_ll
#include <linux/if_ether.h>     // defined ETH_P_ALL

#include "m_error.h"

static int sock;

int create_socket(void)
{

  // int sock;
  if ( (sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1 )
    fatal_error("Error create socket: %s", strerror(errno));


  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 100000; // 1/10 of second
  // set timeout for read in socket
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout)) == -1)
    fatal_error("Error set timeout socket: %s", strerror(errno));

  struct sockaddr_ll my_sock = {0};
  // memset(&my_sock, 0 , sizeof(my_sock));
  my_sock.sll_family = AF_PACKET;
  my_sock.sll_protocol = htons(ETH_P_ALL);
  // my_sock.sll_ifindex = if_nametoindex("lo");
  my_sock.sll_ifindex = 0; // 0 equal all interfaces sniffer


  if (bind(sock, (struct sockaddr *)&my_sock, sizeof(my_sock)) == -1)
    fatal_error("Error bind interface %s", strerror(errno));


  return sock;

}


ssize_t
get_packets(struct sockaddr_ll *link_level,
            unsigned char *buffer,
            const int lenght)
{
  socklen_t link_level_size = sizeof(struct sockaddr_ll);


  ssize_t bytes_received = recvfrom(sock , buffer , lenght , 0,
                          (struct sockaddr *) link_level, &link_level_size);

  // retorna quantidade de bytes farejados
  if (bytes_received >= 0 && bytes_received != -1)
    return bytes_received;

  // recvfrom retornou por conta do timeout definido no socket
  if (bytes_received == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
    return 0;

  if(bytes_received == -1)
    error("Error get packets");

  return -1;

}
