
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

#include <stdint.h>
#include <stdlib.h>      // malloc, free
#include <string.h>      // strncpy
#include <sys/socket.h>  // getnameinfo
#include <netdb.h>       // getnameinfo

#include "domain.h"
#include "thread_pool.h"
#include "sock_util.h"  // check_addr_equal
#include "../jhash.h"
#include "../hashtable.h"

// (2048 * sizeof(struct host)) == ~2.31 MiB cache of domain
#define DEFAULT_CACHE_SIZE 2048

static hashtable_t *ht_hosts = NULL;
static struct host **hosts = NULL;
static size_t cache_size;

static bool
cb_ht_compare ( const void *key1, const void *key2 )
{
  return check_addr_equal ( ( union sockaddr_all * ) key1,
                            ( union sockaddr_all * ) key2 );
}

static hash_t
cb_ht_hash ( const void *key )
{
  union sockaddr_all *addr = ( union sockaddr_all * ) key;

  switch ( addr->sa.sa_family )
    {
      case AF_INET:
        return jhash32 ( ( uint32_t * ) &addr->in.sin_addr,
                         sizeof ( addr->in.sin_addr ),
                         0 );
        break;
      case AF_INET6:
        return jhash32 ( ( uint32_t * ) &addr->in6.sin6_addr,
                         sizeof ( addr->in6.sin6_addr ),
                         0 );
        break;
      default:
        return 0;
    }
}

int
cache_domain_init ( unsigned int size )
{
  cache_size = ( size ) ? size : DEFAULT_CACHE_SIZE;
  hosts = calloc ( cache_size, sizeof ( struct host * ) );

  if ( !hosts )
    return 0;

  ht_hosts = hashtable_new ( cb_ht_hash, cb_ht_compare, free );

  if ( !ht_hosts )
    {
      free ( hosts );
      return 0;
    }

  return 1;
}

// run on thread
static void
ip2domain_exec ( void *arg )
{
  struct host *host = ( struct host * ) arg;

  // convert ipv4 and ipv6
  // if error, convert to text ip same
  if ( getnameinfo ( &host->sa_all.sa,
                     sizeof ( host->sa_all ),
                     host->fqdn,
                     sizeof ( host->fqdn ),
                     NULL,
                     0,
                     NI_DGRAM ) )
    {
      sockaddr_ntop ( &host->sa_all, host->fqdn, sizeof ( host->fqdn ) );
    }

  host->status = RESOLVED;
}

// return:
//  1 name resolved
//  0 name no resolved
// -1 error
int
ip2domain ( union sockaddr_all *sa_all, char *buff, const size_t buff_len )
{
  struct host *host = hashtable_get ( ht_hosts, sa_all );

  if ( host )
    {
      if ( host->status == RESOLVED )  // cache hit
        {
          strncpy ( buff, host->fqdn, buff_len );
          return 1;
        }
      else  // resolving, thread working
        {
          sockaddr_ntop ( sa_all, buff, buff_len );
          return 0;
        }
    }
  else  // cache miss
    {
      sockaddr_ntop ( sa_all, buff, buff_len );

      static size_t index = 0;

      if ( hosts[index] )
        {
          if ( hosts[index]->status == RESOLVING )
            return 0;

          hashtable_remove ( ht_hosts, &hosts[index]->sa_all );
        }
      else
        {
          hosts[index] = malloc ( sizeof ( struct host ) );

          if ( !hosts[index] )
            return -1;
        }

      memcpy ( &hosts[index]->sa_all, sa_all, sizeof ( *sa_all ) );
      hosts[index]->status = RESOLVING;

      // add task to workers (thread pool)
      add_task ( ip2domain_exec, hosts[index] );

      hashtable_set ( ht_hosts, &hosts[index]->sa_all, hosts[index] );

      index = ( index + 1 ) % cache_size;
    }

  return 0;
}

void
cache_domain_free ( void )
{
  if ( !ht_hosts )
    return;

  hashtable_destroy ( ht_hosts );
  free ( hosts );
}
