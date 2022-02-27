
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

#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

/*
 create new vector
 @param nmeb, number of member initial in vector, if 0 default is 16
 @param size, size of one member in array

 @return memory to user or null on error
*/
void *
vector_new ( size_t size_member );

/*
 copy user data into vector
 @oaram mem, pointer returned from vector_new
 @param data, pointer to user data to push in vector

 @return 1 if success or 0 on error
*/
int
vector_push_ ( void **mem, void *data );

#define vector_push( mem, data ) vector_push_ ( ( void ** ) ( &mem ), ( data ) )

/*
  remove last element from vector
  @oaram mem, pointer returned from vector_new

  @return element removed from vector
*/
void *
vector_pop ( void *mem );

/*
  remove all vector elements
  @oaram mem, pointer returned from vector_new
*/
void
vector_clear ( void *mem );

/*
  return number of elements in vector
  @oaram mem, pointer returned from vector_new
*/
size_t
vector_size ( void *mem );

/*
  free vector
  @oaram mem, pointer returned from vector_new
*/
void
vector_free ( void *mem );

#endif  // VECTOR_H
