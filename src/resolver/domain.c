
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
#include "../hashtable.h"

// (2048 * sizeof(struct host)) == ~2.31 MiB cache of domain
#define DEFAULT_CACHE_SIZE 2048

static hashtable_t *ht_hosts = NULL;
static struct host **hosts = NULL;
static size_t cache_size;

static void
cb_ht_free ( void *arg )
{
  free ( arg );
}

static int
cb_ht_compare ( const void *key1, const void *key2 )
{
  return check_addr_equal ( ( void * ) key1, ( void * ) key2 );
}

static inline uint32_t
hash_uint32 ( uint32_t v )
{
  return ( v >> 24 ) ^ ( v >> 16 ) ^ ( v >> 8 ) ^ ( v >> 4 ) ^ v;
}

static hash_t
cb_ht_hash ( const void *key )
{
  struct sockaddr_storage *addr = ( struct sockaddr_storage * ) key;

  switch ( addr->ss_family )
    {
      case AF_INET:
        {
          struct sockaddr_in *sa = ( struct sockaddr_in * ) addr;
          return hash_uint32 ( sa->sin_addr.s_addr );

          break;
        }
      case AF_INET6:
        {
          struct sockaddr_in6 *sa = ( struct sockaddr_in6 * ) addr;

          int i = 4;
          hash_t hash = 0;
          while ( i-- )
            hash ^= hash_uint32 ( sa->sin6_addr.s6_addr32[i] );

          return hash;
        }
    }

  return 0;
}

int
cache_domain_init ( unsigned int size )
{
  cache_size = ( size ) ? size : DEFAULT_CACHE_SIZE;
  hosts = calloc ( cache_size, sizeof ( struct host * ) );

  if ( !hosts )
    return 0;

  ht_hosts = hashtable_new ( cb_ht_hash, cb_ht_compare, cb_ht_free );

  if ( !ht_hosts )
    return 0;

  return 1;
}

// run on thread
static void
ip2domain_exec ( void *arg )
{
  struct host *host = ( struct host * ) arg;

  // convert ipv4 and ipv6
  // if error, convert to text ip same
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
}

// return:
//  1 name resolved
//  0 name no resolved
// -1 error
int
ip2domain ( struct sockaddr_storage *restrict ss,
            char *buff,
            const size_t buff_len )
{
  struct host *host = hashtable_get ( ht_hosts, ss );

  if ( host )
    {
      if ( host->status == RESOLVED )  // cache hit
        {
          strncpy ( buff, host->fqdn, buff_len );
          return 1;
        }
      else  // resolving, thread working
        {
          sockaddr_ntop ( ss, buff, buff_len );
          return 0;
        }
    }
  else  // cache miss
    {
      sockaddr_ntop ( ss, buff, buff_len );

      static size_t index = 0;

      if ( hosts[index] )
        {
          if ( hosts[index]->status == RESOLVING )
            return 0;

          hashtable_remove ( ht_hosts, &hosts[index]->ss );
        }
      else
        {
          hosts[index] = malloc ( sizeof ( struct host ) );

          if ( !hosts[index] )
            return -1;
        }

      memcpy ( &hosts[index]->ss, ss, sizeof ( struct sockaddr_storage ) );
      hosts[index]->status = RESOLVING;

      // add task to workers (thread pool)
      add_task ( ip2domain_exec, hosts[index] );

      hashtable_set ( ht_hosts, &hosts[index]->ss, hosts[index] );

      index = ( index + 1 ) % cache_size;
    }

  return 0;
}

void
cache_domain_free ( void )
{
  if ( ht_hosts )
    hashtable_destroy ( ht_hosts );

  free ( hosts );
}
