
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

#include <stdbool.h>
#include <linux/filter.h>  // struct sock_filter, sock_fprog
#include <sys/socket.h>    // setsockopt

#include "connection.h"
#include "m_error.h"
#include "macro_util.h"

static bool
filter_get ( struct sock_fprog *fprog, const int flags_proto )
{
  // pass only tcp or udp, block net address 127.*
  // suport interface ethernet and tun
  // bpfc -i bpf/program.bpf -f C -p
  static const struct sock_filter ip_tcp_udp[] = {
    { 0x30, 0, 0, 0x00000000 },  { 0x54, 0, 0, 0x000000f0 },
    { 0x15, 0, 9, 0x00000040 },  { 0x20, 0, 0, 0x0000000c },
    { 0x54, 0, 0, 0xff000000 },  { 0x15, 18, 0, 0x7f000000 },
    { 0x20, 0, 0, 0x00000010 },  { 0x54, 0, 0, 0xff000000 },
    { 0x15, 15, 0, 0x7f000000 }, { 0x30, 0, 0, 0x00000009 },
    { 0x15, 12, 0, 0x00000006 }, { 0x15, 11, 0, 0x00000011 },
    { 0x28, 0, 0, 0x0000000c },  { 0x15, 0, 10, 0x00000800 },
    { 0x20, 0, 0, 0x0000001a },  { 0x54, 0, 0, 0xff000000 },
    { 0x15, 7, 0, 0x7f000000 },  { 0x20, 0, 0, 0x0000001e },
    { 0x54, 0, 0, 0xff000000 },  { 0x15, 4, 0, 0x7f000000 },
    { 0x30, 0, 0, 0x00000017 },  { 0x15, 1, 0, 0x00000006 },
    { 0x15, 0, 1, 0x00000011 },  { 0x6, 0, 0, 0x00040000 },
    { 0x6, 0, 0, 0x00000000 },
  };

  // pass only tcp, block net address 127.*
  // suport interface ethernet and tun
  // bpfc -i bpf/program.bpf -f C -p -D TCP
  static const struct sock_filter ip_tcp[] = {
    { 0x30, 0, 0, 0x00000000 },  { 0x54, 0, 0, 0x000000f0 },
    { 0x15, 0, 8, 0x00000040 },  { 0x20, 0, 0, 0x0000000c },
    { 0x54, 0, 0, 0xff000000 },  { 0x15, 16, 0, 0x7f000000 },
    { 0x20, 0, 0, 0x00000010 },  { 0x54, 0, 0, 0xff000000 },
    { 0x15, 13, 0, 0x7f000000 }, { 0x30, 0, 0, 0x00000009 },
    { 0x15, 10, 0, 0x00000006 }, { 0x28, 0, 0, 0x0000000c },
    { 0x15, 0, 9, 0x00000800 },  { 0x20, 0, 0, 0x0000001a },
    { 0x54, 0, 0, 0xff000000 },  { 0x15, 6, 0, 0x7f000000 },
    { 0x20, 0, 0, 0x0000001e },  { 0x54, 0, 0, 0xff000000 },
    { 0x15, 3, 0, 0x7f000000 },  { 0x30, 0, 0, 0x00000017 },
    { 0x15, 0, 1, 0x00000006 },  { 0x6, 0, 0, 0x00040000 },
    { 0x6, 0, 0, 0x00000000 },
  };

  // pass only udp, block net address 127.*
  // suport interface ethernet and tun
  // bpfc -i bpf/program.bpf -f C -p -D UDP
  static const struct sock_filter ip_udp[] = {
    { 0x30, 0, 0, 0x00000000 },  { 0x54, 0, 0, 0x000000f0 },
    { 0x15, 0, 8, 0x00000040 },  { 0x20, 0, 0, 0x0000000c },
    { 0x54, 0, 0, 0xff000000 },  { 0x15, 16, 0, 0x7f000000 },
    { 0x20, 0, 0, 0x00000010 },  { 0x54, 0, 0, 0xff000000 },
    { 0x15, 13, 0, 0x7f000000 }, { 0x30, 0, 0, 0x00000009 },
    { 0x15, 10, 0, 0x00000011 }, { 0x28, 0, 0, 0x0000000c },
    { 0x15, 0, 9, 0x00000800 },  { 0x20, 0, 0, 0x0000001a },
    { 0x54, 0, 0, 0xff000000 },  { 0x15, 6, 0, 0x7f000000 },
    { 0x20, 0, 0, 0x0000001e },  { 0x54, 0, 0, 0xff000000 },
    { 0x15, 3, 0, 0x7f000000 },  { 0x30, 0, 0, 0x00000017 },
    { 0x15, 0, 1, 0x00000011 },  { 0x6, 0, 0, 0x00040000 },
    { 0x6, 0, 0, 0x00000000 },
  };

  switch ( flags_proto )
    {
      case ( TCP | UDP ):
        fprog->len = ARRAY_SIZE ( ip_tcp_udp );
        fprog->filter = ( struct sock_filter * ) ip_tcp_udp;
        break;
      case TCP:
        fprog->len = ARRAY_SIZE ( ip_tcp );
        fprog->filter = ( struct sock_filter * ) ip_tcp;
        break;
      case UDP:
        fprog->len = ARRAY_SIZE ( ip_udp );
        fprog->filter = ( struct sock_filter * ) ip_udp;
        break;
      default:
        ERROR_DEBUG ( "%s", "Protocol filter bpf invalid" );
        return false;
    }

  return true;
}

bool
filter_set ( int sock, const int flags_proto )
{
  struct sock_fprog fprog;

  if ( !filter_get ( &fprog, flags_proto ) )
    return false;

  if ( setsockopt ( sock,
                    SOL_SOCKET,
                    SO_ATTACH_FILTER,
                    &fprog,
                    sizeof ( fprog ) ) == -1 )
    {
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      return false;
    }

  return true;
}
