
/*
 *  Copyright (C) 2020 Mayco S. Berghetti
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

#include <errno.h>  // variable errno
#include <stdbool.h>
#include <stdio.h>
#include <string.h>    // strlen, strerror
#include <linux/in.h>  // IPPROTO_UDP | IPPROTO_TCP

#include "conection.h"
#include "config.h"  // define TCP | UDP
#include "m_error.h"

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

  // switch ( proto )
  //   {
  //     case TCP:
  //       path_file = PATH_TCP;
  //       protocol = IPPROTO_TCP;
  //       break;
  //     case UDP:
  //       path_file = PATH_UDP;
  //       protocol = IPPROTO_UDP;
  //       break;
  //     default:
  //       fatal_error ( "Invalid protocol get conections" );
  //   }

  if ( !( arq = fopen ( path_file, "r" ) ) )
    return -1;

  char *line = NULL;
  size_t len = 0;

  // ignore header in first line
  if ( ( getline ( &line, &len, arq ) ) == -1 )
    {
      free ( line );
      fclose ( arq );
      return -1;
    }

  size_t len_buff_conections = TOT_CONECTIONS_BEGIN;
  *conection = calloc ( len_buff_conections, sizeof ( **conection ) );
  if ( !*conection )
    {
      free ( line );
      fclose ( arq );
      return -1;
    }

  uint32_t count = 0;
  char local_addr[64], rem_addr[64] = {0};

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
                         "%*d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %*X "
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
        fatal_error ( "Error converting ip address: %s", strerror ( errno ) );

      if ( 1 !=
           sscanf ( rem_addr, "%x", &( *conection )[count].remote_address ) )
        fatal_error ( "Error converting ip address: %s", strerror ( errno ) );

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
            fatal_error ( "Alloc memory conections: \"%s\"",
                          strerror ( errno ) );

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

int
get_conections_system ( conection_t **buffer, const int proto )
{
  int tot_con_tcp = 0;
  int tot_con_udp = 0;
  int tot_con = 0;
  int temp = 0;
  int i;

  conection_t *temp_conections_tcp = NULL;
  conection_t *temp_conections_udp = NULL;

  if ( proto & TCP )
    tot_con_tcp = get_info_conections ( &temp_conections_tcp, IPPROTO_TCP );

  if ( proto & UDP )
    tot_con_udp = get_info_conections ( &temp_conections_udp, IPPROTO_UDP );

  if ( tot_con_tcp == -1 || tot_con_udp == -1 )
    fatal_error ( "Error get conections system: %s", strerror ( errno ) );

  tot_con = tot_con_tcp + tot_con_udp;
  *buffer = calloc ( tot_con, sizeof ( **buffer ) );
  if ( !*buffer )
    fatal_error ( "Calloc: %s", strerror ( errno ) );

  for ( i = 0; i < tot_con_tcp; i++ )
    ( *buffer )[i] = temp_conections_tcp[i];

  temp += tot_con_tcp;
  for ( i = 0; i < tot_con_udp; i++ )
    ( *buffer + temp )[i] = temp_conections_udp[i];

  free ( temp_conections_tcp );
  free ( temp_conections_udp );

  return tot_con;
}
