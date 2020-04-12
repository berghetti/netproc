#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <linux/if_packet.h>    // struct sockaddr_ll

#ifndef IP_MAXPACKET
#define IP_MAXPACKET 65535
#endif

// used in parameter of function print_l2()
#define SRC  1
#define DST  2
#define BOTH 3

// values of packet.direction
#define PKT_DOWN 1
#define PKT_UPL  2


struct packet{ // used for function parse_packet
  size_t lenght;
  uint32_t local_address;
  uint16_t local_port;
  uint8_t  direction;
};




// inicializa o socket para escutar conexões
int create_socket();

// pega os dados do socket armazena no buffer e preenche a struct sockaddr_ll
// lenght tamanho maximo do buffer
ssize_t get_packets(struct sockaddr_ll *link_level, unsigned char *buffer, const int lenght);

// aloca os dados brutos nas camadas 2, 3 e 4, tambe verifica se é um pacote de down ou up
int parse_packet(struct packet *pkt, unsigned char *buf, struct sockaddr_ll *ll);


// print address MAC
// flag SRC, DST or BOTH for source, detination ou both address
// void print_l2(struct ethhdr *l2, const int flag);

void print_packet(struct packet *pkt);

#endif //NETWORK_H
