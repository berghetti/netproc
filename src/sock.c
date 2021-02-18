
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>  // malloc
#include <stdbool.h>
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

static bool
socket_setnonblocking ( int sock );

static bool
bind_interface ( int sock, const char *iface );

int
create_socket ( const struct config_op *co )
{
  int sock;

  if ( ( sock = socket ( AF_PACKET, SOCK_RAW, htons ( ETH_P_ALL ) ) ) == -1 )
    {
      ERROR_DEBUG ( "Error create socket: %s", strerror ( errno ) );
      return -1;
    }

  if ( !socket_setnonblocking ( sock ) )
    return -1;

  if ( !bind_interface ( sock, co->iface ) )
    return -1;

  return sock;
}

void
close_socket ( int sock )
{
  if ( sock > 0 )
    close ( sock );
}

static bool
socket_setnonblocking ( int sock )
{
  int flag;

  if ( ( flag = fcntl ( sock, F_GETFL ) ) == -1 )
    {
      ERROR_DEBUG ( "Cannot get socket flags: \"%s\"", strerror ( errno ) );
      return false;
    }

  if ( fcntl ( sock, F_SETFL, flag | O_NONBLOCK ) == -1 )
    {
      ERROR_DEBUG ( "Cannot set socket to non-blocking mode: \"%s\"",
                    strerror ( errno ) );
      return false;
    }

  return true;
}

static bool
bind_interface ( int sock, const char *iface )
{
  struct sockaddr_ll my_sock = {0};
  my_sock.sll_family = AF_PACKET;
  my_sock.sll_protocol = htons ( ETH_P_ALL );

  // 0 match all interfaces
  if ( !iface )
    my_sock.sll_ifindex = 0;
  else
    {
      if ( !( my_sock.sll_ifindex = if_nametoindex ( iface ) ) )
        {
          ERROR_DEBUG ( "%s", strerror ( errno ) );
          return false;
        }
    }

  if ( bind ( sock, ( struct sockaddr * ) &my_sock, sizeof ( my_sock ) ) == -1 )
    {
      ERROR_DEBUG ( "Error bind interface %s", strerror ( errno ) );
      return false;
    }

  return true;
}
