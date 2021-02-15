
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
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

#include <ctype.h>      // isdigit
#include <dirent.h>     // *dir
#include <stdbool.h>    // type boolean
#include <stdint.h>     // type uint*
#include <stdlib.h>     // atoi
#include <string.h>     // memset
#include <sys/types.h>  // *dir

#include <stdio.h>

// len init buffer
#define TOT_PROCESS_BEGIN 512

static bool
is_number ( const char *string );

// recebe o nome de um diretorio, um buffer para armazenar
// os nomes dos diretorios que são numericos e um tamanho maximo do buffer.
// retorna o total de diretorios encontrados, -1 em caso de falha.
int
get_numeric_directory ( uint32_t *restrict buffer,
                        const size_t lenght,
                        const char *restrict path_dir )
{
  DIR *dir;

  if ( ( dir = opendir ( path_dir ) ) == NULL )
    return -1;

  struct dirent *directory = NULL;
  uint32_t count = 0;

  while ( ( directory = readdir ( dir ) ) && count < lenght )
    if ( is_number ( directory->d_name ) )
      buffer[count++] = atoi ( directory->d_name );

  closedir ( dir );

  return count;
}

////////////////////////////////////////////////////////////////////////////////
// version with management of memory auto
int
get_numeric_directory2 ( uint32_t **buffer, const char *path_dir )
{
  DIR *dir;

  if ( ( dir = opendir ( path_dir ) ) == NULL )
    return -1;

  struct dirent *directory = NULL;
  size_t count = 0;

  size_t len_buffer = TOT_PROCESS_BEGIN;
  // *buffer = calloc ( len_buffer, sizeof ( **buffer ) );
  *buffer =
          malloc ( len_buffer * sizeof ( **buffer ) );  // calloc is necessary?
  if ( !*buffer )
    return -1;

  while ( ( directory = readdir ( dir ) ) )
    {
      if ( is_number ( directory->d_name ) )
        ( *buffer )[count++] = atoi ( directory->d_name );

      if ( count == len_buffer )
        {
          // doble len buffer
          len_buffer <<= 1;

          void *temp;
          temp = realloc ( *buffer, len_buffer * sizeof ( **buffer ) );
          // work with data that have
          if ( !temp )
            goto END;
          // return -1;

          *buffer = temp;

          // initialize new space of memory
          // FIXME: its necessary?
          // memset ( &( *buffer )[count],
          //          0,
          //          ( len_buffer - count ) * sizeof ( **buffer ) );
        }
    }

END:
  closedir ( dir );

  return count;
}

static bool
is_number ( const char *string )
{
  while ( *string )
    if ( !isdigit ( *string++ ) )
      return false;

  return true;
}
