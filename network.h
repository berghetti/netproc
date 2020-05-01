#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <linux/if_packet.h>    // struct sockaddr_ll

#ifndef IP_MAXPACKET
#define IP_MAXPACKET 65535      // maximum packet size
#endif

// values of packet.direction
#define PKT_DOWN 1
#define PKT_UPL  2

// used for function parse_packet
struct packet{
  size_t lenght;
  uint32_t local_address;
  uint32_t remote_address;
  uint16_t local_port;
  uint16_t remote_port;
  uint8_t  direction;
};


// organiza os dados recebidosnas camadas 2, 3 e 4 e
// tambem verifica se é um pacote de download(entrada) ou upload(saida)
int parse_packet(struct packet *pkt, unsigned char *buf, struct sockaddr_ll *ll);


#endif //NETWORK_H
