
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

#include <stdlib.h>  // type size_t

// retorna o tamanho da string até null byte ou espaço
// oque ocorrer primeiro
size_t
strlen_space ( const char *string )
{
  size_t n = 0;
  while ( *string && *string++ != ' ' )
    n++;

  return n;
}

// return position of last occurrence of character in string
// char *str, pointer to string
// size_t len, lenght of string
// char ch, character to search
int
find_last_char ( const char *str, size_t len, const char ch )
{
  while ( len-- )
    if ( str[len] == ch )
      return len;

  // not found
  return -1;
}
