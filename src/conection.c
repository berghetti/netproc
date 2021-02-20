
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

#include <errno.h>  // variable errno
#include <stdbool.h>
#include <stdio.h>
#include <string.h>    // strlen, strerror
#include <linux/in.h>  // IPPROTO_UDP | IPPROTO_TCP

#include "conection.h"
#include "config.h"  // define TCP | UDP
#include "m_error.h"

// references
// https://www.kernel.org/doc/Documentation/networking/proc_net_tcp.txt

// caminho do arquivo onde o kernel
// fornece as conexoes TCP e UDP
#define PATH_TCP "/proc/net/tcp"
#define PATH_UDP "/proc/net/udp"

// len initial buffer conections
#define TOT_CONECTIONS_BEGIN 512

// le o arquivo onde fica salva as conexoes '/proc/net/tcp',
// recebe o local do arquivo, um buffer para armazenar
// dados da conexão e o tamanho do buffer,
// retorna a quantidade de registros encontrada
// ou -1 em caso de erro
static int
get_info_conections ( conection_t **conection, const int protocol )
{
  FILE *arq = NULL;
  // char *path_file = NULL;
  const char *path_file = ( protocol == IPPROTO_TCP ) ? PATH_TCP : PATH_UDP;

  if ( !( arq = fopen ( path_file, "r" ) ) )
    {
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      return -1;
    }

  char *line = NULL;
  size_t len = 0;

  // ignore header in first line
  if ( ( getline ( &line, &len, arq ) ) == -1 )
    {
      free ( line );
      fclose ( arq );
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      return -1;
    }

  size_t len_buff_conections = TOT_CONECTIONS_BEGIN;
  *conection = calloc ( len_buff_conections, sizeof ( **conection ) );
  if ( !*conection )
    {
      free ( line );
      fclose ( arq );
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      return -1;
    }

  uint32_t count = 0;
  // char local_addr[64] = {0}, rem_addr[64] = {0};
  char local_addr[9] = {0}, rem_addr[9] = {0};  // enough for ipv4

  unsigned int matches, local_port, rem_port;
  unsigned long int inode;

  while ( ( getline ( &line, &len, arq ) ) != -1 )
    {
      // clang-format off
      // sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
      // 0: 3500007F:0035 00000000:0000 0A 00000000:00000000 00:00000000 00000000   101        0 20911 1 0000000000000000 100 0 0 10 0
      // 1: 0100007F:0277 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 44385 1 0000000000000000 100 0 0 10 0
      // 2: 0100007F:1733 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 27996 1 0000000000000000 100 0 0 10 0
      // clang-format on

      matches = sscanf ( line,
                         "%*d: %8[0-9A-Fa-f]:%X %8[0-9A-Fa-f]:%X %*X "
                         "%*X:%*X %*X:%*X %*X %*d %*d %lu %*512s\n",
                         local_addr,
                         &local_port,
                         rem_addr,
                         &rem_port,
                         &inode );

      if ( matches != 5 )
        return -1;

      if ( 1 !=
           sscanf ( local_addr, "%x", &( *conection )[count].local_address ) )
        {
          ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
          return -1;
        }

      if ( 1 !=
           sscanf ( rem_addr, "%x", &( *conection )[count].remote_address ) )
        {
          ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
          return -1;
        }

      ( *conection )[count].local_port = local_port;
      ( *conection )[count].remote_port = rem_port;
      ( *conection )[count].inode = inode;
      ( *conection )[count].protocol = protocol;

      count++;

      if ( count == len_buff_conections )
        {
          len_buff_conections <<= 1;

          conection_t *temp;
          temp = realloc ( *conection,
                           len_buff_conections * sizeof ( **conection ) );
          if ( !temp )
            {
              ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
              return -1;
            }

          *conection = temp;

          // initialize new space of memory (important)
          memset ( &( *conection )[count],
                   0,
                   ( len_buff_conections - count ) * sizeof ( **conection ) );
        }
    }

  free ( line );
  fclose ( arq );

  return count;
}

// return total conections or -1 on failure
int
get_conections_system ( conection_t **buffer, const int proto )
{
  int tot_con_tcp = 0;
  int tot_con_udp = 0;
  int tot_con;

  conection_t *temp_conections_tcp, *temp_conections_udp;
  conection_t *p_buff, *p_conn;

  if ( proto & TCP )
    {
      if ( -1 == ( tot_con_tcp = get_info_conections ( &temp_conections_tcp,
                                                       IPPROTO_TCP ) ) )
        {
          ERROR_DEBUG ( "%s", "backtrace" );
          return -1;
        }
    }

  if ( proto & UDP )
    {
      if ( -1 == ( tot_con_udp = get_info_conections ( &temp_conections_udp,
                                                       IPPROTO_UDP ) ) )
        {
          ERROR_DEBUG ( "%s", "backtrace" );
          return -1;
        }
    }

  tot_con = tot_con_tcp + tot_con_udp;
  *buffer = calloc ( tot_con, sizeof ( **buffer ) );
  if ( !*buffer )
    {
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      return -1;
    }

  p_buff = *buffer;

  p_conn = temp_conections_tcp;
  while(tot_con_tcp--)
    *p_buff++ = *p_conn++;

  p_conn = temp_conections_udp;
  while(tot_con_udp--)
    *p_buff++ = *p_conn++;

  free ( temp_conections_tcp );
  free ( temp_conections_udp );

  return tot_con;
}
