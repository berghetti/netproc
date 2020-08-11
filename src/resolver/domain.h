#ifndef RESOLVER_H
#define RESOLVER_H

#include <sys/socket.h> // struct sockaddr_storage
#include <netdb.h>      // NI_MAXHOST

struct hosts
{
  struct sockaddr_storage ss;
  char fqdn[NI_MAXHOST];
  int status;
};

// retorna imediatamente o ip em formato de texto, porém na proxima requisição
// irá retornar o dominio que estará em cache (se tudo der certo).
// evitando a latencia que uma consulta DNS pode ter.
int
ip2domain ( struct sockaddr_storage *ss, char *buff, const size_t buff_len );

#endif  // RESOLVER_H
