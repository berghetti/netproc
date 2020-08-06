
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
  //   /*
  //   sudo apt install netsniff-ng, to get program bpfc
  //   bpfc -i filter -f C -p -D TCP (or UDP)
  //   pass ipv4 and tcp
  //   suport to interface ethernet and
  //   interface tun (that not has header ethernet)
  // */
  //
  // #define P_TCP 0x06
  // #define P_UDP 0x11
  //
  // #if UDP
  // #define PROTO P_UDP
  // #elif TCP
  // #define PROTO P_TCP
  // #else
  // #error "protocol missig, use option -D UDP or TCP"
  // #endif
  //
  // /* test in interface tun */
  // l0:   ldb      [0]                      /* load offset */
  // l1:   and      #0xf0                    /* bitwise and */
  // l2:   jeq      #0x40,       l3,	l11     /* if ipv4 true */
  // l3:   ld       [12]                     /* test source ip */
  // l4:   and      #0xff000000
  // l5:   jeq      #0x7f000000, drop, l6    /* drop network 127.0.0.0/8 */
  // l6:   ld       [16]                     /* test dest ip */
  // l7:   and      #0xff000000
  // l8:   jeq      #0x7f000000, drop, l9    /* drop network 127.0.0.0/8 */
  // l9:   ldb      [9]                      /* load offset */
  // l10:  jeq      #PROTO,   pass, l11      /* if protocol true */
  // /* test in interface ethernet */
  // l11:  ldh      [12]                     /* load half-word offset 12 */
  // l12:  jeq      #0x0800,     l13, drop   /* if ipv4 */
  // l13:  ld       [26]                     /* test source ip */
  // l14:  and      #0xff000000
  // l15:  jeq      #0x7f000000, drop, l16   /* drop network 127.0.0.0/8 */
  // l16:  ld       [30]                     /* test dest ip */
  // l17:  and      #0xff000000
  // l18:  jeq      #0x7f000000, drop, l19   /* drop network 127.0.0.0/8 */
  // l19:  ldb      [23]
  // l20:  jeq      #PROTO,    pass, drop    /* if protocol true */
  // pass: ret      #262144
  // drop: ret      #0

  // pass only ipv4, tcp or udp (choose this),
  // and all network, except 127.0.0.0/8,
  // suport interface ethernet and tun

  struct sock_filter ip_tcp[] = {
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

  struct sock_filter ip_udp[] = {
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
