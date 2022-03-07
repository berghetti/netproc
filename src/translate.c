
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

#include <stdio.h>  // snprintf
#include <string.h>
#include <stdbool.h>
#include <netdb.h>      // NI_MAXHOST, NI_MAXSERV
#include <arpa/inet.h>  // htons
#include <linux/in.h>   // IPPROTO_UDP | IPPROTO_TCP

#include "translate.h"
#include "sockaddr.h"
#include "resolver/sock_util.h"
#include "resolver/resolver.h"

#define LEN_TUPLE ( ( NI_MAXHOST + NI_MAXSERV ) * 2 ) + 7 + 10

char *
translate ( const connection_t *con, const struct config_op *co )
{
  // NOTE: structs members should be zered

  // local_socket
  union sockaddr_all l_sock = { .in = { .sin_family = AF_INET,
                                        .sin_port = con->tuple.l4.local_port,
                                        .sin_addr.s_addr =
                                                con->tuple.l3.local.ip } };

  // remote_socket
  union sockaddr_all r_sock = { .in = { .sin_family = AF_INET,
                                        .sin_port = con->tuple.l4.remote_port,
                                        .sin_addr.s_addr =
                                                con->tuple.l3.remote.ip } };

  char l_host[NI_MAXHOST], r_host[NI_MAXHOST];

  if ( co->translate_host )
    {
      ip2domain ( &l_sock, l_host, sizeof ( l_host ) );
      ip2domain ( &r_sock, r_host, sizeof ( r_host ) );
    }
  else
    {
      sockaddr_ntop ( &l_sock, l_host, sizeof ( l_host ) );
      sockaddr_ntop ( &r_sock, r_host, sizeof ( r_host ) );
    }

  char l_service[NI_MAXSERV], r_service[NI_MAXSERV];

  if ( co->translate_service )
    {
      const char *proto =
              ( con->tuple.l4.protocol == IPPROTO_UDP ) ? "udp" : "tcp";

      if ( !port2serv ( l_sock.in.sin_port,
                        proto,
                        l_service,
                        sizeof ( l_service ) ) )
        snprintf ( l_service,
                   sizeof ( l_service ),
                   "%u",
                   con->tuple.l4.local_port );

      if ( !port2serv ( r_sock.in.sin_port,
                        proto,
                        r_service,
                        sizeof ( r_service ) ) )
        snprintf ( r_service,
                   sizeof ( r_service ),
                   "%u",
                   con->tuple.l4.remote_port );
    }
  else
    {
      snprintf ( l_service,
                 sizeof ( l_service ),
                 "%u",
                 con->tuple.l4.local_port );
      snprintf ( r_service,
                 sizeof ( r_service ),
                 "%u",
                 con->tuple.l4.remote_port );
    }

  // tuple ip:port <-> ip:port
  static char tuple[LEN_TUPLE];

  snprintf ( tuple,
             sizeof ( tuple ),
             "%s:%s <-> %s:%s",
             l_host,
             l_service,
             r_host,
             r_service );

  return tuple;
}
