#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <linux/tcp.h>
#include <netpacket/packet.h>

// used in parameter of function print_l2()
#define SRC  1
#define DST  2
#define BOTH 3

#define PKT_DOWN 1
#define PKT_UPL  2

extern int sock;

struct packet{ // used for function parse_packet
  uint32_t local_address;
  uint16_t local_port;
  uint32_t lenght;
  uint8_t  direction;
};




// inicializa o socket para escutar conexões
int create_socket();

// pega os dados do socket armazena no buffer e preenche a struct sockaddr_ll
// lenght tamanho maximo do buffer
ssize_t get_packets(struct sockaddr_ll *link_level, unsigned char *buffer, const int lenght);

// aloca os dados brutos nas camadas 2, 3 e 4, tambe verifica se é um pacote de down ou up
int parse_packet(struct sockaddr_ll *ll, unsigned char *buf, struct packet *pkt);


// print address MAC
// flag SRC, DST or BOTH for source, detination ou both address
void print_l2(struct ethhdr *l2, const int flag);

void print_packet(struct packet *pkt);

#endif
