#ifndef SOCK_SNIFF_H
#define SOCK_SNIFF_H

extern int sock;

// inicializa o socket para escutar conexões
int
create_socket ( void );

void
close_socket ( void );

#endif  // SOCK_SNIFF_H
