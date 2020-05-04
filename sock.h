#ifndef SOCK_SNIFF_H
#define SOCK_SNIFF_H

#include <linux/if_packet.h>  // struct sockaddr_ll

// inicializa o socket para escutar conexões
int
create_socket ();

// pega os dados do socket armazena no buffer e preenche a struct sockaddr_ll
// lenght tamanho maximo do buffer
ssize_t
get_packets ( struct sockaddr_ll *link_level,
              unsigned char *buffer,
              const int lenght );

#endif  // SOCK_SNIFF_H
