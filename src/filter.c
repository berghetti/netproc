
/*
 *  Copyright (C) 2020 Mayco S. Berghetti
 *
 *
 *  This program is free software: you can redistribute it and/or modify
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


#include <assert.h>
#include <linux/filter.h>
#include <sys/socket.h>  // setsockopt

#include "config.h"
#include "m_error.h"

#define ELEMENTS_ARRAY( x ) ( sizeof ( x ) / sizeof ( x[0] ) )

void
set_filter ( int sock, const struct config_op *co )
{
  // tcpdump not net 127 and ip and tcp -s 100 -dd
  // block 127.0.0.0/8 network
  // pass ip, udp
  struct sock_filter ip_tcp[] = {
          {0x28, 0, 0, 0x0000000c},
          {0x15, 0, 9, 0x00000800},
          {0x20, 0, 0, 0x0000001a},
          {0x54, 0, 0, 0xff000000},
          {0x15, 6, 0, 0x7f000000},
          {0x20, 0, 0, 0x0000001e},
          {0x54, 0, 0, 0xff000000},
          {0x15, 3, 0, 0x7f000000},
          {0x30, 0, 0, 0x00000017},
          {0x15, 0, 1, 0x00000006},
          {0x6, 0, 0, 0x00040000},
          {0x6, 0, 0, 0x00000000},
  };

  // tcpdump not net 127.0 and ip and udp -s 100 -dd
  // block 127.0.0.0/8 network
  // pass ip, tcp
  struct sock_filter ip_udp[] = {
          {0x28, 0, 0, 0x0000000c},
          {0x15, 0, 9, 0x00000800},
          {0x20, 0, 0, 0x0000001a},
          {0x54, 0, 0, 0xffff0000},
          {0x15, 6, 0, 0x7f000000},
          {0x20, 0, 0, 0x0000001e},
          {0x54, 0, 0, 0xffff0000},
          {0x15, 3, 0, 0x7f000000},
          {0x30, 0, 0, 0x00000017},
          {0x15, 0, 1, 0x00000011},
          {0x6, 0, 0, 0x00040000},
          {0x6, 0, 0, 0x00000000},
  };

  struct sock_fprog bpf;

  if ( co->udp )
    {
      bpf.len = ELEMENTS_ARRAY ( ip_udp );
      bpf.filter = ip_udp;
    }
  else
    {
      bpf.len = ELEMENTS_ARRAY ( ip_tcp );
      bpf.filter = ip_tcp;
    }

  if ( setsockopt (
               sock, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof ( bpf ) ) ==
       -1 )
    fatal_error ( "config filter: %s", strerror ( errno ) );
}
