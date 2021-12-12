
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

#define ENTRY_SIZE_BUF 128

// -1 failure
int
get_numeric_directory ( uint32_t **buffer, const char *path_dir )
{
  DIR *dir;
  if ( ( dir = opendir ( path_dir ) ) == NULL )
    {
      ERROR_DEBUG ( "%s - %s", path_dir, strerror ( errno ) );
      return -1;
    }

  errno = 0;
  int count = 0;
  int len_buffer = 0;
  struct dirent *directory;
  while ( ( directory = readdir ( dir ) ) )
    {
      if ( count == len_buffer )
        {
          len_buffer += ENTRY_SIZE_BUF;
          void *temp;
          temp = realloc ( *buffer, len_buffer * sizeof ( **buffer ) );

          if ( !temp )
            {
              errno = 0;
              goto END;
            }

          *buffer = temp;
        }

      char crap;
      int rs = sscanf ( directory->d_name, "%u%c", &( *buffer )[count], &crap );

      if ( rs != 1 )
        continue;

      count++;
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
