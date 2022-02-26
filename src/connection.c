
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

// references
// https://www.kernel.org/doc/Documentation/networking/proc_net_tcp.txt

#include <errno.h>  // variable errno
#include <stdbool.h>
#include <stdio.h>        // FILE *
#include <string.h>       // strlen, strerror
#include <linux/in.h>     // IPPROTO_UDP | IPPROTO_TCP
#include <netinet/tcp.h>  // TCP_ESTABLISHED, TCP_TIME_WAIT...

#include "connection.h"
#include "hashtable.h"
#include "config.h"  // define TCP | UDP
#include "m_error.h"

static hashtable_t *ht_connections = NULL;

// TODO:create new hash functions later.
// see
// https://elixir.bootlin.com/linux/v5.10.19/source/include/linux/jhash.h#L117
static hash_t
cb_hash ( const void *key )
{
  unsigned long int k = ( unsigned long int ) FROM_PTR ( key );

  return ( k >> 24 ) ^ ( k >> 16 ) ^ ( k >> 8 ) ^ ( k >> 4 ) ^ k;
}

static int
cb_compare ( const void *key1, const void *key2 )
{
  return ( key1 == key2 );
}

static int
connection_update_ ( const char *path_file, const int protocol )
{
  FILE *arq = fopen ( path_file, "r" );
  if ( !arq )
    {
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      return 0;
    }

  int ret = 1;
  char *line = NULL;
  size_t len = 0;
  // ignore header in first line
  if ( ( getline ( &line, &len, arq ) ) == -1 )
    {
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      ret = 0;
      goto EXIT;
    }

  while ( ( getline ( &line, &len, arq ) ) != -1 )
    {
      // clang-format off
      // sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
      // 0: 3500007F:0035 00000000:0000 0A 00000000:00000000 00:00000000 00000000   101        0 20911 1 0000000000000000 100 0 0 10 0
      // 1: 0100007F:0277 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 44385 1 0000000000000000 100 0 0 10 0
      // 2: 0100007F:1733 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 27996 1 0000000000000000 100 0 0 10 0
      // clang-format on

      char local_addr[10], rem_addr[10];  // enough for ipv4
      unsigned long int inode;
      uint16_t local_port, rem_port;
      uint8_t state;
      int rs = sscanf ( line,
                        "%*d: %9[0-9A-Fa-f]:%hX %9[0-9A-Fa-f]:%hX %hhX"
                        " %*X:%*X %*X:%*X %*X %*d %*d %lu %*512s\n",
                        local_addr,
                        &local_port,
                        rem_addr,
                        &rem_port,
                        &state,
                        &inode );

      if ( rs != 6 )
        {
          ERROR_DEBUG ( "Error read file conections\"%s\"",
                        strerror ( errno ) );
          ret = 0;
          goto EXIT;
        }

      // ignore this conections
      if ( state == TCP_TIME_WAIT || state == TCP_LISTEN )
        continue;

      connection_t *conn = hashtable_get ( ht_connections, TO_PTR ( inode ) );

      // conn already in hashtable
      if ( conn )
        continue;

      conn = malloc ( sizeof ( connection_t ) );
      if ( !conn )
        {
          ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
          ret = 0;
          goto EXIT;
        }

      memset ( &conn->net_stat, 0, sizeof ( struct net_stat ) );
      hashtable_set ( ht_connections, TO_PTR ( inode ), conn );

      rs = sscanf ( local_addr, "%x", &conn->local_address );
      if ( rs != 1 )
        {
          ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
          ret = 0;
          goto EXIT;
        }

      rs = sscanf ( rem_addr, "%x", &conn->remote_address );
      if ( rs != 1 )
        {
          ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
          ret = 0;
          goto EXIT;
        }

      conn->local_port = local_port;
      conn->remote_port = rem_port;
      conn->state = state;
      conn->inode = inode;
      conn->protocol = protocol;
    }

EXIT:
  free ( line );
  fclose ( arq );

  return ret;
}

int
connection_init ( void )
{
  ht_connections = hashtable_new ( cb_hash, cb_compare, free );

  return ( NULL != ht_connections );
}

#define PATH_TCP "/proc/net/tcp"
#define PATH_UDP "/proc/net/udp"

int
connection_update ( const int proto )
{
  if ( proto & TCP )
    {
      if ( !connection_update_ ( PATH_TCP, IPPROTO_TCP ) )
        return 0;
    }

  if ( proto & UDP )
    {
      if ( !connection_update_ ( PATH_UDP, IPPROTO_UDP ) )
        return 0;
    }

  return 1;
}

connection_t *
connection_get ( const unsigned long int inode )
{
  return hashtable_get ( ht_connections, TO_PTR ( inode ) );
}

void
connection_free ( void )
{
  if ( ht_connections )
    hashtable_destroy ( ht_connections );
}
