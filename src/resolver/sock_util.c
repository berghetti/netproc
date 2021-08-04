
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
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

#include <stddef.h>      // size_t
#include <string.h>      // memcmp
#include <arpa/inet.h>   // inet_ntop
#include <sys/socket.h>  // struct sockaddr_storage

int
check_addr_equal ( struct sockaddr_storage *addr1,
                   struct sockaddr_storage *addr2 )
{
  if ( addr1->ss_family != addr2->ss_family )
    return 0;

  switch ( addr1->ss_family )
    {
      case AF_INET:
        {
          struct sockaddr_in *sa1 = ( struct sockaddr_in * ) addr1;
          struct sockaddr_in *sa2 = ( struct sockaddr_in * ) addr2;

          return ( sa1->sin_addr.s_addr == sa2->sin_addr.s_addr );

          break;
        }
      case AF_INET6:
        {
          struct sockaddr_in6 *sa1 = ( struct sockaddr_in6 * ) addr1;
          struct sockaddr_in6 *sa2 = ( struct sockaddr_in6 * ) addr2;

          return ( memcmp ( &sa1->sin6_addr,
                            &sa2->sin6_addr,
                            sizeof ( sa1->sin6_addr ) ) == 0 );
        }
    }

  return 0;
}

char *
sockaddr_ntop ( struct sockaddr_storage *addr,
                char *buf,
                const size_t len_buff )
{
  const char *ret;

  switch ( addr->ss_family )
    {
      case AF_INET:
        ret = inet_ntop ( AF_INET,
                          &( ( struct sockaddr_in * ) addr )->sin_addr,
                          buf,
                          len_buff );
        break;
      case AF_INET6:
        ret = inet_ntop ( AF_INET6,
                          &( ( struct sockaddr_in6 * ) addr )->sin6_addr,
                          buf,
                          len_buff );
        break;
      default:
        ret = NULL;
    }

  return ( char * ) ret;
}
