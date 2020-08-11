
#include <arpa/inet.h>  // inet_ntop
#include <stdlib.h>
#include <stdbool.h>

#include "../m_error.h"

bool
check_addr_equal ( struct sockaddr_storage *addr1,
                   struct sockaddr_storage *addr2 )
{
  if ( addr1->ss_family != addr2->ss_family )
    return false;

  switch ( addr1->ss_family )
    {
      case AF_INET:
        {
          struct sockaddr_in *sa1 = ( struct sockaddr_in * ) addr1;
          struct sockaddr_in *sa2 = ( struct sockaddr_in * ) addr2;

          if ( sa1->sin_addr.s_addr != sa2->sin_addr.s_addr )
            return false;

          return true;
        }
      case AF_INET6:
        {
          struct sockaddr_in6 *sa1 = ( struct sockaddr_in6 * ) addr1;
          struct sockaddr_in6 *sa2 = ( struct sockaddr_in6 * ) addr2;

          if ( memcmp ( &sa1->sin6_addr,
                        &sa2->sin6_addr,
                        sizeof ( sa1->sin6_addr ) ) != 0 )
            return false;

          return true;
        }
    }

  return false;
}

int
sockaddr_ntop ( struct sockaddr_storage *addr,
                char *buf,
                const size_t len_buff )
{
  const char *ret = NULL;

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
    }

  if ( !ret )
    {
      error ( "inet_ntop: \"%s\" ", strerror ( errno ) );
      return 0;
    }

  return 1;
}
