
#include <string.h>           // strerror
#include <errno.h>            // variable errno
#include <arpa/inet.h>        // htons
#include <linux/if_ether.h>   // defined ETH_P_ALL
#include <linux/if_packet.h>  // struct sockaddr_ll
#include <net/if.h>           // if_nametoindex
#include <sys/socket.h>       // socket
#include <sys/types.h>        // socket
#include <unistd.h>           // close

#include "m_error.h"

// socket timeout default in microseconds
// 1 second          <--> 1E+6 microseconds
// 1E+5 microseconds <--> 1/10 second
#define TIMEOUT 1E+5

int sock;

// defined in main.c
extern char *iface;

static void
bind_interface ( const char *iface );

static void
set_timeout( void );

int
create_socket ( void )
{
  if ( ( sock = socket ( AF_PACKET, SOCK_RAW, htons ( ETH_P_ALL ) ) ) == -1 )
    fatal_error ( "Error create socket: %s", strerror ( errno ) );

  set_timeout();

  bind_interface ( iface );

  return sock;
}

void
close_socket ( void )
{
  if ( sock > 0 )
    close ( sock );
}

static void
set_timeout( void )
{
  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = TIMEOUT;

  // set timeout for read in socket
  if ( setsockopt ( sock,
                    SOL_SOCKET,
                    SO_RCVTIMEO,
                    &read_timeout,
                    sizeof ( read_timeout ) ) == -1 )
    fatal_error ( "Error set timeout socket: %s", strerror ( errno ) );
}

static void
bind_interface ( const char *iface )
{
  struct sockaddr_ll my_sock = {0};
  my_sock.sll_family = AF_PACKET;
  my_sock.sll_protocol = htons ( ETH_P_ALL );

  // 0 equal all interfaces
  if ( !iface )
    my_sock.sll_ifindex = 0;
  else
    {
      if ( !( my_sock.sll_ifindex = if_nametoindex ( iface ) ) )
        fatal_error ( "Interface: %s", strerror ( errno ) );
    }

  if ( bind ( sock, ( struct sockaddr * ) &my_sock, sizeof ( my_sock ) ) == -1 )
    fatal_error ( "Error bind interface %s", strerror ( errno ) );
}
