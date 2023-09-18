
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
#include <netinet/tcp.h>  // TCP_ESTABLISHED, TCP_TIME_WAIT...

#include "connection.h"
#include "hashtable.h"
#include "jhash.h"
#include "config.h"  // define TCP | UDP
#include "m_error.h"
#include "macro_util.h"

static hashtable_t *ht_connections = NULL;

static hash_t
hash ( const void *key, size_t size )
{
  return jhash8 ( key, size, 0 );
}

static bool
ht_cb_compare_inode ( const void *key1, const void *key2 )
{
  if ( key1 == key2 )
    return true;

  return ( *( unsigned long * ) key1 == *( unsigned long * ) key2 );
}

static bool
ht_cb_compare_tuple ( const void *key1, const void *key2 )
{
  if ( key1 == key2 )
    return true;

  return ( 0 == memcmp ( key1, key2, sizeof ( struct tuple ) ) );
}

#define MARK_ACTIVE_CON( conn ) ( ( conn )->refs_active = 3 )

static connection_t *
create_new_conn ( unsigned long inode,
                  char *local_addr,
                  char *remote_addr,
                  uint16_t local_port,
                  uint16_t rem_port,
                  uint8_t state,
                  int protocol )
{
  /* using calloc to ensure that struct tuple is clean
     necessary to struct net_stat */
  connection_t *conn = calloc ( 1, sizeof *conn );
  if ( !conn )
    {
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      return NULL;
    }

  int rs = sscanf ( local_addr, "%x", &conn->tuple.l3.local.ip );

  if ( 1 != rs )
    goto EXIT_ERROR;

  rs = sscanf ( remote_addr, "%x", &conn->tuple.l3.remote.ip );

  if ( 1 != rs )
    goto EXIT_ERROR;

  conn->tuple.l4.local_port = local_port;
  conn->tuple.l4.remote_port = rem_port;
  conn->tuple.l4.protocol = protocol;
  conn->state = state;
  conn->inode = inode;

  /* each conn has two entries on hashtable,
   this is used to remove inactives conns and free only one time a conn */
  MARK_ACTIVE_CON ( conn );

  return conn;

EXIT_ERROR:
  ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
  free ( conn );
  return NULL;
}

static void
connection_insert ( connection_t *conn )
{
  // make one entries in hashtable to same connection
  hashtable_min_set (
          ht_connections,
          conn,
          &conn->inode,
          hash ( &conn->inode, SIZEOF_MEMBER ( connection_t, inode ) ) );

  hashtable_min_set (
          ht_connections,
          conn,
          &conn->tuple,
          hash ( &conn->tuple, SIZEOF_MEMBER ( connection_t, tuple ) ) );
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
      /* clang-format off
      sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
      0: 3500007F:0035 00000000:0000 0A 00000000:00000000 00:00000000 00000000   101        0 20911 1 0000000000000000 100 0 0 10 0
      1: 0100007F:0277 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 44385 1 0000000000000000 100 0 0 10 0
      2: 0100007F:1733 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 27996 1 0000000000000000 100 0 0 10 0
      clang-format on */

      char local_addr[10], rem_addr[10];  // enough for ipv4
      unsigned long inode;
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

      // TODO: need check to tuple here? linux recycling inode?
      connection_t *conn = connection_get_by_inode ( inode );

      if ( conn )
        {
          MARK_ACTIVE_CON ( conn );
          continue;
        }

      conn = create_new_conn ( inode,
                               local_addr,
                               rem_addr,
                               local_port,
                               rem_port,
                               state,
                               protocol );
      if ( !conn )
        {
          ret = 0;
          goto EXIT;
        }

      connection_insert ( conn );
    }

EXIT:
  free ( line );
  fclose ( arq );

  return ret;
}

static int
remove_inactives_conns ( UNUSED hashtable_t *ht,
                         void *value,
                         UNUSED void *user_data )
{
  connection_t *conn = value;

  switch ( conn->refs_active-- )
    {
      case 1:
        return 1;

      case 0:
        free ( conn );
        return 1;

      default:
        return 0;
    }
}

bool
connection_init ( void )
{
  ht_connections = hashtable_min_new ();

  return !!ht_connections;
}

#define PATH_TCP "/proc/net/tcp"
#define PATH_UDP "/proc/net/udp"

bool
connection_update ( const int proto )
{
  hashtable_foreach_remove ( ht_connections, remove_inactives_conns, NULL );

  if ( proto & TCP )
    {
      if ( !connection_update_ ( PATH_TCP, IPPROTO_TCP ) )
        return false;
    }

  if ( proto & UDP )
    {
      if ( !connection_update_ ( PATH_UDP, IPPROTO_UDP ) )
        return false;
    }

  return true;
}

connection_t *
connection_get_by_inode ( const unsigned long inode )
{
  return hashtable_min_get ( ht_connections,
                             &inode,
                             hash ( &inode, sizeof ( inode ) ),
                             ht_cb_compare_inode );
}

connection_t *
connection_get_by_tuple ( struct tuple *tuple )
{
  return hashtable_min_get (
          ht_connections,
          tuple,
          hash ( tuple, SIZEOF_MEMBER ( connection_t, tuple ) ),
          ht_cb_compare_tuple );
}

static void
conn_free ( void *data )
{
  connection_t *conn = data;

  /* first entry go be 0, not free, remove second entry */
  if ( conn->refs_exit++ == 1 )
    free ( conn );
}

void
connection_free ( void )
{
  if ( ht_connections )
    hashtable_min_detroy ( ht_connections, conn_free );
}
