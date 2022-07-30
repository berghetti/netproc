
/*
 *  Copyright (C) 2020-2022 Mayco S. Berghetti
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

#include <stdbool.h>
#include <stddef.h>      // size_t
#include <string.h>      // memcmp
#include <arpa/inet.h>   // inet_ntop
#include <sys/socket.h>  // struct sockaddr_storage

#include "../sockaddr.h"

bool
check_addr_equal ( union sockaddr_all *addr1, union sockaddr_all *addr2 )
{
  if ( addr1->sa.sa_family == addr2->sa.sa_family )
    {
      switch ( addr1->sa.sa_family )
        {
          case AF_INET:
            return ( addr1->in.sin_addr.s_addr == addr2->in.sin_addr.s_addr );
          case AF_INET6:
            return ( 0 == memcmp ( &addr1->in6.sin6_addr,
                                   &addr2->in6.sin6_addr,
                                   sizeof ( addr1->in6.sin6_addr ) ) );
        }
    }

  return false;
}

void
sockaddr_ntop ( union sockaddr_all *addr, char *buf, const size_t len_buff )
{
  switch ( addr->sa.sa_family )
    {
      case AF_INET:
        inet_ntop ( AF_INET, &addr->in.sin_addr, buf, len_buff );
        break;
      case AF_INET6:
        inet_ntop ( AF_INET6, &addr->in6.sin6_addr, buf, len_buff );
    }
}
