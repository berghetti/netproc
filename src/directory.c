
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

#include <ctype.h>      // isdigit
#include <dirent.h>     // *dir
#include <stdbool.h>    // type boolean
#include <stdint.h>     // type uint*
#include <stdlib.h>     // atoi
#include <sys/types.h>  // *dir

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

static bool
is_number ( const char *string )
{
  while ( *string )
    if ( !isdigit ( *string++ ) )
      return false;

  return true;
}
