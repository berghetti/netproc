
#include <stdio.h>  // snprintf
#include <stdbool.h>
#include <sys/socket.h>  // getnameinfo
#include <netdb.h>       // getnameinfo
#include <arpa/inet.h>   // htons

#include "translate.h"

// defined in main.c
extern bool udp;
extern bool translate_host;
extern bool translate_service;

#define LEN_TUPLE ( ( NI_MAXHOST + NI_MAXSERV ) * 2 ) + 7

static void
check_flags ( int * );

char *
translate ( const conection_t *con )
{
  // tuple ip:port <-> ip:port
  static char tuple[LEN_TUPLE];
  struct sockaddr_in l_sock, r_sock;  // local_socket and remote_socket
  const socklen_t socklen = sizeof ( struct sockaddr_in );

  int flags = 0;
  check_flags ( &flags );

  char l_host[NI_MAXHOST], l_service[NI_MAXSERV];
  char r_host[NI_MAXHOST], r_service[NI_MAXSERV];

  l_sock.sin_family = AF_INET;
  r_sock.sin_family = AF_INET;

  l_sock.sin_port = htons ( con->local_port );
  l_sock.sin_addr.s_addr = con->local_address;

  r_sock.sin_port = htons ( con->remote_port );
  r_sock.sin_addr.s_addr = con->remote_address;

  getnameinfo ( ( struct sockaddr * ) &r_sock,
                socklen,
                r_host,
                NI_MAXHOST,
                r_service,
                NI_MAXSERV,
                flags );

  getnameinfo ( ( struct sockaddr * ) &l_sock,
                socklen,
                l_host,
                NI_MAXHOST,
                l_service,
                NI_MAXSERV,
                flags );

  snprintf ( tuple,
             LEN_TUPLE,
             "%s:%s <-> %s:%s",
             l_host,
             l_service,
             r_host,
             r_service );

  return tuple;
}

static void
check_flags ( int *flags )
{
  *flags = 0;

  if ( udp )
    *flags |= NI_DGRAM;

  if ( !translate_host && !translate_service )
    *flags |= NI_NUMERICHOST | NI_NUMERICSERV;
  else if ( !translate_host )
    *flags |= NI_NUMERICHOST;
  else if ( !translate_service )
    *flags |= NI_NUMERICSERV;
}
