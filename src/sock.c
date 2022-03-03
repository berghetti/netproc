
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>           // malloc
#include <arpa/inet.h>        // htons
#include <errno.h>            // variable errno
#include <linux/if_ether.h>   // defined ETH_P_ALL
#include <linux/if_packet.h>  // struct sockaddr_ll
#include <net/if.h>           // if_nametoindex
#include <string.h>           // strerror
#include <sys/socket.h>       // socket
#include <sys/types.h>        // socket
#include <fcntl.h>            // fcntl
#include <unistd.h>           // close

#include "sock.h"
#include "m_error.h"

static int
socket_setnonblocking ( int sock )
{
  int flag;

  if ( ( flag = fcntl ( sock, F_GETFL ) ) == -1 )
    {
      ERROR_DEBUG ( "Cannot get socket flags: \"%s\"", strerror ( errno ) );
      return 0;
    }

  if ( fcntl ( sock, F_SETFL, flag | O_NONBLOCK ) == -1 )
    {
      ERROR_DEBUG ( "Cannot set socket to non-blocking mode: \"%s\"",
                    strerror ( errno ) );
      return 0;
    }

  return 1;
}

static int
bind_interface ( int sock, const char *iface )
{
  struct sockaddr_ll sock_ll = {
    .sll_family = AF_PACKET,
    .sll_protocol = htons ( ETH_P_ALL ),
    .sll_ifindex = 0  // explicit 0 match all interfaces
  };

  if ( iface )
    {
      if ( !( sock_ll.sll_ifindex = if_nametoindex ( iface ) ) )
        {
          ERROR_DEBUG ( "%s", strerror ( errno ) );
          return 0;
        }
    }

  if ( bind ( sock, ( struct sockaddr * ) &sock_ll, sizeof ( sock_ll ) ) == -1 )
    {
      ERROR_DEBUG ( "Error bind interface %s", strerror ( errno ) );
      return 0;
    }

  return 1;
}

int
socket_init ( const char *iface )
{
  int sock;

  if ( ( sock = socket ( AF_PACKET, SOCK_RAW, htons ( ETH_P_ALL ) ) ) == -1 )
    {
      ERROR_DEBUG ( "Error create socket: %s", strerror ( errno ) );
      return -1;
    }

  if ( !socket_setnonblocking ( sock ) )
    goto ERROR_EXIT;

  if ( !bind_interface ( sock, iface ) )
    goto ERROR_EXIT;

  return sock;

ERROR_EXIT:

  close ( sock );
  return -1;
}

void
socket_free ( int sock )
{
  if ( sock > 0 )
    close ( sock );
}
