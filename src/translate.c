
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
#include "resolver/sock_util.h"
#include "resolver/resolver.h"

#define LEN_TUPLE ( ( NI_MAXHOST + NI_MAXSERV ) * 2 ) + 7 + 10

char *
translate ( const conection_t *con, const struct config_op *co )
{
  // tuple ip:port <-> ip:port
  static char tuple[LEN_TUPLE];
  struct sockaddr_in l_sock, r_sock;  // local_socket and remote_socket

  const char *proto = ( con->protocol == IPPROTO_UDP ) ? "udp" : "tcp";

  char l_host[NI_MAXHOST], l_service[NI_MAXSERV];
  char r_host[NI_MAXHOST], r_service[NI_MAXSERV];

  l_sock.sin_family = AF_INET;
  l_sock.sin_port = con->local_port;
  l_sock.sin_addr.s_addr = con->local_address;

  r_sock.sin_family = AF_INET;
  r_sock.sin_port = con->remote_port;
  r_sock.sin_addr.s_addr = con->remote_address;

  if ( co->translate_host )
    {
      ip2domain ( ( struct sockaddr_storage * ) &l_sock,
                  l_host,
                  sizeof ( l_host ) );
      ip2domain ( ( struct sockaddr_storage * ) &r_sock,
                  r_host,
                  sizeof ( r_host ) );
    }
  else
    {
      sockaddr_ntop ( ( struct sockaddr_storage * ) &l_sock,
                      l_host,
                      sizeof ( l_host ) );
      sockaddr_ntop ( ( struct sockaddr_storage * ) &r_sock,
                      r_host,
                      sizeof ( r_host ) );
    }

  if ( co->translate_service )
    {
      if ( !port2serv (
                   l_sock.sin_port, proto, l_service, sizeof ( l_service ) ) )
        snprintf ( l_service, sizeof ( l_service ), "%u", con->local_port );

      if ( !port2serv (
                   r_sock.sin_port, proto, r_service, sizeof ( r_service ) ) )
        snprintf ( r_service, sizeof ( r_service ), "%u", con->remote_port );
    }
  else
    {
      snprintf ( l_service, sizeof ( l_service ), "%u", con->local_port );
      snprintf ( r_service, sizeof ( r_service ), "%u", con->remote_port );
    }

  snprintf ( tuple,
             sizeof ( tuple ),
             "%s:%s <-> %s:%s",
             l_host,
             l_service,
             r_host,
             r_service );

  return tuple;
}
