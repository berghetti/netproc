
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
#include "conection.h"
#include "m_error.h"

#define ELEMENTS_ARRAY( x ) ( sizeof ( x ) / sizeof ( x[0] ) )

void
set_filter ( int sock, const struct config_op *co )
{
  // pass only tcp or udp, block net address 127.*
  // suport interface ethernet and tun
  // bpfc -i bpf/program.bpf -f C -p
  struct sock_filter ip_tcp_udp[] = {
          {0x30, 0, 0, 0x00000000},  {0x54, 0, 0, 0x000000f0},
          {0x15, 0, 9, 0x00000040},  {0x20, 0, 0, 0x0000000c},
          {0x54, 0, 0, 0xff000000},  {0x15, 18, 0, 0x7f000000},
          {0x20, 0, 0, 0x00000010},  {0x54, 0, 0, 0xff000000},
          {0x15, 15, 0, 0x7f000000}, {0x30, 0, 0, 0x00000009},
          {0x15, 12, 0, 0x00000006}, {0x15, 11, 0, 0x00000011},
          {0x28, 0, 0, 0x0000000c},  {0x15, 0, 10, 0x00000800},
          {0x20, 0, 0, 0x0000001a},  {0x54, 0, 0, 0xff000000},
          {0x15, 7, 0, 0x7f000000},  {0x20, 0, 0, 0x0000001e},
          {0x54, 0, 0, 0xff000000},  {0x15, 4, 0, 0x7f000000},
          {0x30, 0, 0, 0x00000017},  {0x15, 1, 0, 0x00000006},
          {0x15, 0, 1, 0x00000011},  {0x6, 0, 0, 0x00040000},
          {0x6, 0, 0, 0x00000000},
  };

  // pass only tcp, block net address 127.*
  // suport interface ethernet and tun
  // bpfc -i bpf/program.bpf -f C -p -D TCP
  struct sock_filter ip_tcp[] = {
          {0x30, 0, 0, 0x00000000},  {0x54, 0, 0, 0x000000f0},
          {0x15, 0, 8, 0x00000040},  {0x20, 0, 0, 0x0000000c},
          {0x54, 0, 0, 0xff000000},  {0x15, 16, 0, 0x7f000000},
          {0x20, 0, 0, 0x00000010},  {0x54, 0, 0, 0xff000000},
          {0x15, 13, 0, 0x7f000000}, {0x30, 0, 0, 0x00000009},
          {0x15, 10, 0, 0x00000006}, {0x28, 0, 0, 0x0000000c},
          {0x15, 0, 9, 0x00000800},  {0x20, 0, 0, 0x0000001a},
          {0x54, 0, 0, 0xff000000},  {0x15, 6, 0, 0x7f000000},
          {0x20, 0, 0, 0x0000001e},  {0x54, 0, 0, 0xff000000},
          {0x15, 3, 0, 0x7f000000},  {0x30, 0, 0, 0x00000017},
          {0x15, 0, 1, 0x00000006},  {0x6, 0, 0, 0x00040000},
          {0x6, 0, 0, 0x00000000},
  };

  // pass only udp, block net address 127.*
  // suport interface ethernet and tun
  // bpfc -i bpf/program.bpf -f C -p -D UDP
  struct sock_filter ip_udp[] = {
          {0x30, 0, 0, 0x00000000},  {0x54, 0, 0, 0x000000f0},
          {0x15, 0, 8, 0x00000040},  {0x20, 0, 0, 0x0000000c},
          {0x54, 0, 0, 0xff000000},  {0x15, 16, 0, 0x7f000000},
          {0x20, 0, 0, 0x00000010},  {0x54, 0, 0, 0xff000000},
          {0x15, 13, 0, 0x7f000000}, {0x30, 0, 0, 0x00000009},
          {0x15, 10, 0, 0x00000011}, {0x28, 0, 0, 0x0000000c},
          {0x15, 0, 9, 0x00000800},  {0x20, 0, 0, 0x0000001a},
          {0x54, 0, 0, 0xff000000},  {0x15, 6, 0, 0x7f000000},
          {0x20, 0, 0, 0x0000001e},  {0x54, 0, 0, 0xff000000},
          {0x15, 3, 0, 0x7f000000},  {0x30, 0, 0, 0x00000017},
          {0x15, 0, 1, 0x00000011},  {0x6, 0, 0, 0x00040000},
          {0x6, 0, 0, 0x00000000},
  };

  struct sock_fprog bpf;

  switch ( co->proto )
    {
      case ( TCP | UDP ):
        bpf.len = ELEMENTS_ARRAY ( ip_tcp_udp );
        bpf.filter = ip_tcp_udp;
        break;
      case TCP:
        bpf.len = ELEMENTS_ARRAY ( ip_tcp );
        bpf.filter = ip_tcp;
        break;
      case UDP:
        bpf.len = ELEMENTS_ARRAY ( ip_udp );
        bpf.filter = ip_udp;
        break;
      default:
        fatal_error ( "Protocol filter bpf invalid" );
    }

  if ( setsockopt (
               sock, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof ( bpf ) ) ==
       -1 )
    fatal_error ( "config filter: %s", strerror ( errno ) );
}
