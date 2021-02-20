
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

#ifndef STR_H
#define STR_H

#include <stdlib.h>  // type size_t

// retorna o tamanho da string até null byte ou espaço
// oque ocorrer primeiro
size_t
strlen_space ( const char *string );

// retorna a posição do ultimo caracter pesquisado,
// ou -1 caso não encontre
int
find_last_char ( const char *str, const char ch, size_t len );

#endif  // STR_H
