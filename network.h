#ifndef NETWORK_H
#define NETWORK_H

// user in parameter  function print_l2()
#define SRC  1
#define DST  2
#define BOTH 3

extern int sock;

#include <netpacket/packet.h>

// inicializa o socket para escutar conexões
int create_socket();

// pega os dados do socket armazena no buffer e preenche a struct sockaddr_ll
// lenght tamanho maximo do buffer
int get_packets(struct sockaddr_ll *link_level, unsigned char *buffer, const int lenght);

// print address MAC
// flag SRC, DST or BOTH for source, detination ou both address
void print_l2(struct ethhdr *l2, const int flag);

#endif
