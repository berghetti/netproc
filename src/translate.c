
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

#include <stdio.h>  // snprintf
#include <string.h>
#include <stdbool.h>
#include <netdb.h>      // NI_MAXHOST, NI_MAXSERV
#include <arpa/inet.h>  // htons

#include "translate.h"
#include "resolver/sock_util.h"
#include "resolver/domain.h"
#include "resolver/service.h"

#define LEN_TUPLE ( ( NI_MAXHOST + NI_MAXSERV ) * 2 ) + 7 + 10

// static void
// check_flags ( int *restrict flags, const struct config_op *restrict co );

char *
translate ( const conection_t *restrict con,
            const struct config_op *restrict co )
{
  // tuple ip:port <-> ip:port
  static char tuple[LEN_TUPLE] = { 0 };
  struct sockaddr_in l_sock, r_sock;  // local_socket and remote_socket

  char *proto = ( co->proto & UDP ) ? "udp" : "tcp";

  char l_host[NI_MAXHOST], l_service[NI_MAXSERV];
  char r_host[NI_MAXHOST], r_service[NI_MAXSERV];

  l_sock.sin_family = AF_INET;
  l_sock.sin_port = htons ( con->local_port );
  l_sock.sin_addr.s_addr = con->local_address;

  r_sock.sin_family = AF_INET;
  r_sock.sin_port = htons ( con->remote_port );
  r_sock.sin_addr.s_addr = con->remote_address;

  if ( co->translate_host )
    {
      ip2domain ( ( struct sockaddr_storage * ) &l_sock, l_host, NI_MAXHOST );
      ip2domain ( ( struct sockaddr_storage * ) &r_sock, r_host, NI_MAXHOST );
    }
  else
    {
      sockaddr_ntop (
              ( struct sockaddr_storage * ) &l_sock, l_host, NI_MAXHOST );
      sockaddr_ntop (
              ( struct sockaddr_storage * ) &r_sock, r_host, NI_MAXHOST );
    }

  if ( co->translate_service )
    {
      port2serv ( l_sock.sin_port, proto, l_service, NI_MAXSERV );
      port2serv ( r_sock.sin_port, proto, r_service, NI_MAXSERV );
    }
  else
    {
      snprintf ( l_service, NI_MAXSERV, "%u", con->local_port );
      snprintf ( r_service, NI_MAXSERV, "%u", con->remote_port );
    }

  snprintf ( tuple,
             LEN_TUPLE,
             "%s:%s <-> %s:%s",
             l_host,
             l_service,
             r_host,
             r_service );

  return tuple;
}
