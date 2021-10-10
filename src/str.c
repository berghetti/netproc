
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

#include <sys/types.h>  // type ssize_t
#include <string.h>     // strrchr

size_t
strlen_space ( const char *str )
{
  char *p = strchr ( str, ' ' );
  if ( p )
    return p - str;

  return strlen ( str );
}

ssize_t
index_last_char ( const char *str, const int ch )
{
  char *p = strrchr ( str, ch );
  if ( p )
    return p - str;

  // not found
  return -1;
}
