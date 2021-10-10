
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

// le o arquivo onde fica salva as conexoes '/proc/net/tcp',
// recebe o local do arquivo, um buffer para armazenar
// dados da conexão e o tamanho do buffer,
// retorna a quantidade de registros encontrada
// ou -1 em caso de erro
static int
get_info_conections ( conection_t **conection,
                      const int protocol,
                      const char *path_file )
{
  FILE *arq = fopen ( path_file, "r" );
  if ( !arq )
    {
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      return -1;
    }

  size_t count;

  char *line = NULL;
  size_t len = 0;
  // ignore header in first line
  if ( ( getline ( &line, &len, arq ) ) == -1 )
    {
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      count = -1;
      goto EXIT;
    }

  count = 0;
  *conection = NULL;
  size_t buff_size = 0;
  while ( ( getline ( &line, &len, arq ) ) != -1 )
    {
      if ( count == buff_size )
        {
          buff_size += ENTRY_SIZE;

          conection_t *temp;
          temp = realloc ( *conection, buff_size * sizeof ( **conection ) );
          if ( !temp )
            {
              ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
              free ( *conection );
              count = -1;
              goto EXIT;
            }

          *conection = temp;

          // initialize new space of memory (important)
          memset ( &( *conection )[count],
                   0,
                   ( buff_size - count ) * sizeof ( **conection ) );
        }

      // clang-format off
      // sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
      // 0: 3500007F:0035 00000000:0000 0A 00000000:00000000 00:00000000 00000000   101        0 20911 1 0000000000000000 100 0 0 10 0
      // 1: 0100007F:0277 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 44385 1 0000000000000000 100 0 0 10 0
      // 2: 0100007F:1733 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 27996 1 0000000000000000 100 0 0 10 0
      // clang-format on

      char local_addr[10], rem_addr[10];  // enough for ipv4
      unsigned int local_port, rem_port, state;
      unsigned long int inode;

      int rs = sscanf ( line,
                        "%*d: %9[0-9A-Fa-f]:%X %9[0-9A-Fa-f]:%X %X"
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
          count = -1;
          goto EXIT;
        }

      // ignore this conections
      if ( state == TCP_TIME_WAIT || state == TCP_LISTEN )
        continue;

      rs = sscanf ( local_addr, "%x", &( *conection )[count].local_address );
      if ( rs != 1 )
        {
          ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
          count = -1;
          goto EXIT;
        }

      rs = sscanf ( rem_addr, "%x", &( *conection )[count].remote_address );
      if ( rs != 1 )
        {
          ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
          count = -1;
          goto EXIT;
        }

      ( *conection )[count].local_port = local_port;
      ( *conection )[count].remote_port = rem_port;
      ( *conection )[count].state = state;
      ( *conection )[count].inode = inode;
      ( *conection )[count].protocol = protocol;

      count++;
    }

EXIT:
  free ( line );
  fclose ( arq );

  return count;
}

// return total conections or -1 on failure
int
get_conections ( conection_t **buffer, const int proto )
{
  int tot_con_tcp = 0;
  int tot_con_udp = 0;
  int tot_con;

  conection_t *temp_conections_tcp = NULL, *temp_conections_udp = NULL;
  conection_t *p_buff, *p_conn;

  if ( proto & TCP )
    {
      if ( -1 == ( tot_con_tcp = get_info_conections (
                           &temp_conections_tcp, IPPROTO_TCP, PATH_TCP ) ) )
        {
          ERROR_DEBUG ( "%s", "backtrace" );
          tot_con = -1;
          goto EXIT;
        }
    }

  if ( proto & UDP )
    {
      if ( -1 == ( tot_con_udp = get_info_conections (
                           &temp_conections_udp, IPPROTO_UDP, PATH_UDP ) ) )
        {
          ERROR_DEBUG ( "%s", "backtrace" );
          tot_con = -1;
          goto EXIT;
        }
    }

  tot_con = tot_con_tcp + tot_con_udp;
  *buffer = calloc ( tot_con, sizeof ( **buffer ) );
  if ( !*buffer )
    {
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      tot_con = -1;
      goto EXIT;
    }

  p_buff = *buffer;

  p_conn = temp_conections_tcp;
  while ( tot_con_tcp-- )
    *p_buff++ = *p_conn++;

  p_conn = temp_conections_udp;
  while ( tot_con_udp-- )
    *p_buff++ = *p_conn++;

EXIT:
  free ( temp_conections_tcp );
  free ( temp_conections_udp );

  return tot_con;
}
