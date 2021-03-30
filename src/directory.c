
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

#include <stdio.h>
#include <errno.h>
#include <ctype.h>      // isdigit
#include <dirent.h>     // *dir
#include <stdbool.h>    // type boolean
#include <stdint.h>     // type uint*
#include <stdlib.h>     // malloc
#include <string.h>     // memset
#include <sys/types.h>  // *dir

#include "m_error.h"

// len init buffer
#define TOT_PROCESS_BEGIN 256

// retorna o total de diretorios encontrados, -1 em caso de falha.
int
get_numeric_directory ( uint32_t **buffer, const char *path_dir )
{
  DIR *dir;
  struct dirent *directory = NULL;
  size_t count = 0;
  size_t len_buffer = TOT_PROCESS_BEGIN;
  char crap;

  if ( ( dir = opendir ( path_dir ) ) == NULL )
    {
      ERROR_DEBUG ( "%s - %s", path_dir, strerror ( errno ) );
      return -1;
    }

  // FIXME: calloc is necessary?
  // *buffer = calloc ( len_buffer, sizeof ( **buffer ) );
  *buffer = malloc ( len_buffer * sizeof ( **buffer ) );
  if ( !*buffer )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      count = -1;
      goto END;
    }

  errno = 0;
  while ( ( directory = readdir ( dir ) ) )
    {
      if ( 1 !=
           ( sscanf (
                   directory->d_name, "%u%c", &( *buffer )[count], &crap ) ) )
        continue;

      if ( ++count == len_buffer )
        {
          // doble len buffer
          len_buffer <<= 1;

          void *temp;
          temp = realloc ( *buffer, len_buffer * sizeof ( **buffer ) );
          // work with data that have
          if ( !temp )
            {
              errno = 0;
              goto END;
            }

          *buffer = temp;

          // FIXME: its necessary?
          // initialize only new space of memory
          // memset ( &( *buffer )[count],
          //          0,
          //          ( len_buffer - count ) * sizeof ( **buffer ) );
        }
    }

  if ( errno )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      count = -1;
    }

END:
  closedir ( dir );

  return count;
}
