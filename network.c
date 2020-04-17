
#include <stdbool.h>            // boolean type
#include <string.h>             // strerror
#include <errno.h>              // variable errno
#include <sys/types.h>          // socket
#include <sys/socket.h>         // socket
#include <arpa/inet.h>          // htons
#include <net/if.h>             // if_nametoindex
#include <linux/if_packet.h>    // struct sockaddr_ll
#include <linux/if_ether.h>     // struct ethhdr
#include <linux/ip.h>           // struct iphdr

#include <stdio.h>    // provisorio

#include "network.h"
#include "m_error.h"

// bit more fragments do cabeçalho IP
#define IP_MF 0x2000

// mascara para testar o offset do fragmento
#define IP_OFFMASK 0x1FFF


// retorna verdadeiro se o bit esta ligado no byte
#define TEST_BITS(x,bits) ((x) & (bits))

static int sock;

// defined in main
extern bool udp;

// Aproveitamos do fato dos cabeçalhos TCP e UDP
// receberem as portas de origem e destino na mesma ordem,
// e como atributos iniciais, assim podemos utilizar esse estrutura
// simplificada para extrair as portas tanto de pacotes
// TCP quanto UDP, lembrando que não utilizaremos outros campos
// dos cabeçalhos.
struct tcp_udp_h
{
  uint16_t source;
  uint16_t dest;
};

void insert_data_packet(struct packet *pkt,
                        const uint8_t  direction,
                        const uint32_t local_address,
                        const uint32_t remote_address,
                        const uint16_t local_port,
                        const uint16_t remote_port);

int is_frag(struct iphdr *l3);


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

  if (bytes_received >= 0 && bytes_received != -1)
    return bytes_received;

  if (bytes_received == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
    return 0;

  if(bytes_received == -1)
    error("Error get packets");

  return -1;

}

struct pkt_ip_fragment
{
  uint16_t source_port;
  uint16_t dest_port;
  uint16_t pkt_id;
  uint8_t id_ar;
  uint8_t ttl;
};

static struct pkt_ip_fragment pkt_ip_frag[10] = {0};

int
parse_packet(struct packet *pkt,
            unsigned char *buf,
            struct sockaddr_ll *ll)
{
  struct ethhdr *l2;
  struct iphdr  *l3;
  struct tcp_udp_h *l4;


  l2 = (struct ethhdr *) buf;
  if(ntohs(l2->h_proto) != ETH_P_IP)
    // not is a packet internet protocol
    return 0;

  l3 = (struct iphdr *) (buf + ETH_HLEN);


  if (l3->protocol == IPPROTO_TCP && !udp)
    // caso tenha farejado pacotes TCP e opção udp nao estaja ligada. Default
    l4 = (struct tcp_udp_h *) (buf + ETH_HLEN + (l3->ihl * 4));
  else if (l3->protocol == IPPROTO_UDP && udp)
    // caso tenha farejado pacote UDP e a opção udp esteja ligada
    l4 = (struct tcp_udp_h *) (buf + ETH_HLEN + (l3->ihl * 4));
  else
    // pacote não suportado
    return 0;

  // testa se o bit more fragments (MF) esta ligado
  // e se o offset é 0, caso sim, esse é o primeiro fragmento
  if ( (ntohs(l3->frag_off) & IP_MF) &&
      ((ntohs(l3->frag_off) & IP_OFFMASK) == 0) )
    {
      // puts("primeiro pacote do seguimento");
      for (size_t i = 0; i < 10; i++)
        {
          if ( pkt_ip_frag[i].id_ar == 0 ||
               pkt_ip_frag[i].ttl == 0 )
            {
              pkt_ip_frag[i].pkt_id = l3->id;
              pkt_ip_frag[i].source_port = ntohs(l4->source);
              pkt_ip_frag[i].dest_port = ntohs(l4->dest);
              pkt_ip_frag[i].id_ar = i;
              pkt_ip_frag[i].ttl = 10;
            }
        }
    }

    // void insert_data_packet(const char *direction,
    //                         const uint32_t local_address,
    //                         const uint32_t remote_address,
    //                         const uint16_t local_port,
    //                         const uint16_t remote_port)

  int id = -1;
  // create packet
  if(ll->sll_pkttype == PACKET_OUTGOING )
    { // upload
      if ( (id = is_frag(l3)) != -1 )
        insert_data_packet(pkt, PKT_UPL, l3->saddr, l3->daddr, pkt_ip_frag[id].source_port, pkt_ip_frag[id].dest_port);
      else
        insert_data_packet(pkt, PKT_UPL, l3->saddr, l3->daddr, ntohs(l4->source), ntohs(l4->dest));

      // pkt->direction = PKT_UPL;
      // pkt->local_address = l3->saddr;
      // pkt->remote_address = l3->daddr;
      // pkt->local_port = ntohs(l4->source);
      // pkt->remote_port = ntohs(l4->dest);
    }
  else
    { // download
      if ( (id = is_frag(l3)) != -1 )
        insert_data_packet(pkt, PKT_DOWN, l3->daddr, l3->saddr, pkt_ip_frag[id].dest_port, pkt_ip_frag[id].source_port);
      else
        insert_data_packet(pkt, PKT_DOWN, l3->daddr, l3->saddr, ntohs(l4->dest), ntohs(l4->source));

      // pkt->direction = PKT_DOWN;
      // pkt->local_address = l3->daddr;
      // pkt->remote_address = l3->saddr;
      // pkt->local_port = ntohs(l4->dest);
      // pkt->remote_port = ntohs(l4->source);
    }

  return 1;

}

int is_frag(struct iphdr *l3)
{
  for (size_t i = 0; i < 10; i++)
    {
      if (pkt_ip_frag[i].pkt_id == l3->id && pkt_ip_frag[i].ttl > 0)
        {
          // printf("fragmento port %d\n", pkt_ip_frag[i].source_port);
          if ( (ntohs(l3->frag_off) & IP_MF) == 0 )
            // se for o  ultimo fragmento
            pkt_ip_frag[i].id_ar = 0;

          pkt_ip_frag[i].ttl--;
          return i;
        }
    }
    return -1;
}


void insert_data_packet(struct packet *pkt,
                        const uint8_t  direction,
                        const uint32_t local_address,
                        const uint32_t remote_address,
                        const uint16_t local_port,
                        const uint16_t remote_port)
{
  pkt->direction = direction;
  pkt->local_address = local_address;
  pkt->remote_address = remote_address;
  pkt->local_port = local_port;
  pkt->remote_port = remote_port;

  // printf("inserido porta local - %d\n", pkt->local_port);
}


// void print_l2(struct ethhdr *l2, const int flag)
// {
//   if (flag == SRC || flag == BOTH)
//     {
//       printf("MAC SRC: %02x:%02x:%02x:%02x:%02x:%02x\n",
//             l2->h_source[0],
//             l2->h_source[1],
//             l2->h_source[2],
//             l2->h_source[3],
//             l2->h_source[4],
//             l2->h_source[5]);
//     }
//   if (flag == DST || flag == BOTH)
//     {
//       printf("MAC DST: %02x:%02x:%02x:%02x:%02x:%02x\n",
//             l2->h_dest[0],
//             l2->h_dest[1],
//             l2->h_dest[2],
//             l2->h_dest[3],
//             l2->h_dest[4],
//             l2->h_dest[5]);
//     }
// }

// void print_packet(struct packet *pkt)
// {
//
//   char buf_ip[INET_ADDRSTRLEN];
//   printf("IP local    -> %s\n", inet_ntop(AF_INET, &pkt->local_address, buf_ip, INET_ADDRSTRLEN));
//   // printf("IP remote -> %s\n", inet_ntop(AF_INET, pkt->daddr, buf_ip, INET_ADDRSTRLEN));
//
//   printf("PORT local  -> %d\n", pkt->local_port);
//   // printf("PORT remote -> %d\n", ntohs(pkt->dest));
//
// }
