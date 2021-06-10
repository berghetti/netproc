
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

#include <stdlib.h>      // free
#include <string.h>      // strncpy
#include <sys/socket.h>  // getnameinfo
#include <netdb.h>       // getnameinfo

#include "domain.h"
#include "thread_pool.h"
#include "list.h"
#include "sock_util.h"  // check_addr_equal

// (2048 * sizeof(struct host)) == ~2.31 MiB cache of domain
#define DEFAULT_CACHE_ENTRIES 2048
static unsigned int max_cache_size = DEFAULT_CACHE_ENTRIES;
static struct list list_hosts = { 0 };

void
cache_domain_init ( unsigned int size )
{
  if ( size )
    max_cache_size = size;
}

void
cache_domain_free ( void )
{
  struct list_node *node, *temp;

  node = list_hosts.head;
  while ( node )
    {
      temp = node->next;

      free ( node->data );
      free ( node );

      node = temp;
    }
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

static struct host *
create_host ( struct sockaddr_storage *ss )
{
  struct host *host = malloc ( sizeof *host );

  if ( host )
    {
      memcpy ( &host->ss, ss, sizeof ( *ss ) );
      host->status = RESOLVING;
    }

  return host;
}

static struct host *
search_host ( struct sockaddr_storage *ss )
{
  struct list_node *node = list_hosts.head;

  while ( node )
    {
      if ( check_addr_equal ( node->data, ss ) )
        return node->data;

      node = node->next;
    }

  return NULL;
}

// return:
//  1 name resolved
//  0 name no resolved
int
ip2domain ( struct sockaddr_storage *ss, char *buff, const size_t buff_len )
{
  struct host *host = search_host ( ss );

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
      // transform binary to text
      sockaddr_ntop ( ss, buff, buff_len );

      // add task to workers (thread pool)
      host = create_host ( ss );
      add_task ( ip2domain_exec, host );
      list_push ( &list_hosts, host );

      if ( list_hosts.size > max_cache_size )
        {
          host = list_hosts.tail->data;
          if ( host->status == RESOLVED )
            {
              free ( host );
              list_delete ( &list_hosts, list_hosts.tail );
            }
        }
    }

  return 0;
}
