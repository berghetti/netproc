
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

#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "domain.h"
#include "sock_util.h"
#include "../m_error.h"

// (2048 * sizeof(struct host)) == 2.26 MiB cache of domain
#define MAX_CACHE_ENTRIES 2048

#define RESOLVED 1
#define RESOLVING 2

// circular index
#define UPDATE_INDEX_CACHE( idx ) \
  ( ( idx ) = ( ( idx ) + 1 ) % MAX_CACHE_ENTRIES )

#define UPDATE_TOT_HOSTS_IN_CACHE( tot )                    \
  ( ( tot ) = ( ( tot ) < MAX_CACHE_ENTRIES ) ? ( tot ) + 1 \
                                              : MAX_CACHE_ENTRIES )

static int
check_name_resolved ( struct sockaddr_storage *ss,
                      struct hosts *hosts_cache,
                      const size_t tot_hosts_cache )
{
  for ( size_t i = 0; i < tot_hosts_cache; i++ )
    {
      if ( check_addr_equal ( ss, &hosts_cache[i].ss ) )
        {
          if ( hosts_cache[i].status == RESOLVED )
            return i;  // cache hit

          // already in cache, but not resolved
          return -2;
        }
    }

  // not found in cache
  return -1;
}

void *
ip2domain_thread ( void *arg )
{
  struct hosts *host = ( struct hosts * ) arg;

  // convert ipv4 and ipv6
  if ( getnameinfo ( ( struct sockaddr * ) &host->ss,
                     sizeof ( host->ss ),
                     host->fqdn,
                     sizeof ( host->fqdn ),
                     NULL,
                     0,
                     NI_DGRAM ) )
    {
      sockaddr_ntop ( &host->ss, host->fqdn, sizeof ( host->fqdn ) );
    }

  host->status = RESOLVED;

  pthread_exit ( NULL );
}

int
ip2domain ( struct sockaddr_storage *ss, char *buff, const size_t buff_len )
{
  static struct hosts hosts_cache[MAX_CACHE_ENTRIES];
  static unsigned int tot_hosts_cache = 0;
  static unsigned int index_cache_host = 0;

  pthread_t tid;
  pthread_attr_t attr;

  int nr = check_name_resolved ( ss, hosts_cache, tot_hosts_cache );
  if ( nr >= 0 )
    {
      // cache hit
      strncpy ( buff, hosts_cache[nr].fqdn, buff_len );
      return 1;
    }
  else if ( nr == -2 )
    {
      // already resolving
      sockaddr_ntop ( ss, buff, buff_len );
      return 0;
    }
  else
    {
      // cache miss

      // if status equal RESOLVING, a thread already working this slot,
      // go to next
      unsigned int count = 0;
      while ( hosts_cache[index_cache_host].status == RESOLVING )
        {
          UPDATE_INDEX_CACHE ( index_cache_host );

          // no buffer space currently available, return ip in format of text.
          // increase the buffer size if you want ;)
          if ( count++ == MAX_CACHE_ENTRIES - 1 )
            {
              sockaddr_ntop ( ss, buff, buff_len );
              return 0;
            }
        }

      memcpy ( &hosts_cache[index_cache_host].ss, ss, sizeof ( *ss ) );

      hosts_cache[index_cache_host].status = RESOLVING;

      // transform binary to text
      sockaddr_ntop ( ss, buff, buff_len );

      pthread_attr_init ( &attr );
      pthread_attr_setstacksize ( &attr, 256 * 1024 );  // stack size 256KiB
      pthread_attr_setdetachstate ( &attr, PTHREAD_CREATE_DETACHED );

      // passes buffer space for thread to work
      if ( pthread_create ( &tid,
                            &attr,
                            ip2domain_thread,
                            ( void * ) &hosts_cache[index_cache_host] ) )
        ERROR_DEBUG ( "pthread_create: \"%s\"", strerror ( errno ) );

      UPDATE_TOT_HOSTS_IN_CACHE ( tot_hosts_cache );
      UPDATE_INDEX_CACHE ( index_cache_host );
    }

  return 0;
}
