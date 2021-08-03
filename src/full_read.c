
/*
 *  Copyright (C) 2021 Mayco S. Berghetti
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

#include <errno.h>   // errno
#include <stdlib.h>  // realloc
#include <unistd.h>  // read

#define ENTRY_SIZE_BUFF 64

ssize_t
full_read ( const int fd, char **buffer )
{
  size_t count = 0;
  size_t buff_size = 0;
  ssize_t total_read = 0;
  char *p_buf;

  *buffer = NULL;
  while ( 1 )
    {
      if ( !count )
        {
          count = ENTRY_SIZE_BUFF;
          buff_size += ENTRY_SIZE_BUFF;
          char *t = realloc ( *buffer, buff_size );
          if ( !t )
            goto ERROR_EXIT;

          *buffer = t;
          p_buf = t + total_read;
        }

      ssize_t bytes_read = read ( fd, p_buf, count );

      if ( bytes_read == -1 )
        {
          if ( errno != EINTR )
            goto ERROR_EXIT;

          continue;
        }
      else if ( bytes_read )
        {
          p_buf += bytes_read;
          count -= bytes_read;
          total_read += bytes_read;
        }
      else
        break;  // EOF
    }

  return total_read;

ERROR_EXIT:

  free ( *buffer );
  return -1;
}
