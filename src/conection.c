
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

#include "conection.h"
#include "config.h"  // define TCP | UDP
#include "m_error.h"

// caminho do arquivo onde o kernel
// fornece as conexoes TCP e UDP
#define PATH_TCP "/proc/net/tcp"
#define PATH_UDP "/proc/net/udp"

#define ENTRY_SIZE 64

static int
get_info_conections ( conection_t **conection,
                      size_t *cur_size,
                      size_t *cur_count,
                      const int protocol,
                      const char *path_file )
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
      if ( *cur_count == *cur_size )
        {
          *cur_size += ENTRY_SIZE;

          conection_t *temp;
          temp = realloc ( *conection, *cur_size * sizeof ( **conection ) );
          if ( !temp )
            {
              ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
              ret = 0;
              goto EXIT;
            }

          *conection = temp;

          // initialize new space of memory (important)
          memset ( &( *conection )[*cur_count],
                   0,
                   ( *cur_size - *cur_count ) * sizeof ( **conection ) );
        }

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

      rs = sscanf (
              local_addr, "%x", &( *conection )[*cur_count].local_address );
      if ( rs != 1 )
        {
          ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
          ret = 0;
          goto EXIT;
        }

      rs = sscanf (
              rem_addr, "%x", &( *conection )[*cur_count].remote_address );
      if ( rs != 1 )
        {
          ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
          ret = 0;
          goto EXIT;
        }

      ( *conection )[*cur_count].local_port = local_port;
      ( *conection )[*cur_count].remote_port = rem_port;
      ( *conection )[*cur_count].state = state;
      ( *conection )[*cur_count].inode = inode;
      ( *conection )[*cur_count].protocol = protocol;

      ( *cur_count )++;
    }

EXIT:
  free ( line );
  fclose ( arq );

  return ret;
}

// return total conections or -1 on failure
int
get_conections ( conection_t **buffer, const int proto )
{
  *buffer = NULL;
  size_t cur_size = 0, cur_count = 0;

  if ( proto & TCP )
    {
      if ( !get_info_conections (
                   buffer, &cur_size, &cur_count, IPPROTO_TCP, PATH_TCP ) )
        {
          goto ERROR_EXIT;
        }
    }

  if ( proto & UDP )
    {
      if ( !get_info_conections (
                   buffer, &cur_size, &cur_count, IPPROTO_UDP, PATH_UDP ) )
        {
          goto ERROR_EXIT;
        }
    }

  return cur_count;

ERROR_EXIT:
  free ( *buffer );
  return -1;
}
