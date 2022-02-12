
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

#include <stdlib.h>  // realloc, free
#include <string.h>  // memcpy

struct vector
{
  size_t elements_allocated;
  size_t elements_used;
  size_t element_size;
};

/* CHUNK is header, MEM is user memory */

#define CHUNCK_TO_MEM( pos ) ( ( char * ) ( pos ) + sizeof ( struct vector ) )
#define MEM_TO_CHUNK( pos ) \
  ( ( struct vector * ) ( ( char * ) ( pos ) - sizeof ( struct vector ) ) )

static void *
vector_alloc ( struct vector *v, size_t size )
{
  return realloc ( v, size + sizeof ( struct vector ) );
}

static void
vector_copy ( struct vector *restrict v, void *restrict data )
{
  char *ptr = CHUNCK_TO_MEM ( v ) + ( v->elements_used * v->element_size );

  memcpy ( ptr, data, v->element_size );
}

#define LEN_INIT_VECTOR 16

void *
vector_new ( size_t size_member )
{
  struct vector *vt = vector_alloc ( NULL, LEN_INIT_VECTOR * size_member );

  if ( vt )
    {
      vt->elements_allocated = LEN_INIT_VECTOR;
      vt->elements_used = 0;
      vt->element_size = size_member;

      return CHUNCK_TO_MEM ( vt );
    }

  return vt;
}

int
vector_push_ ( void **restrict mem, void *restrict data )
{
  struct vector *vt = MEM_TO_CHUNK ( *mem );

  if ( vt->elements_used == vt->elements_allocated )
    {
      vt->elements_allocated <<= 1;
      size_t new_size = vt->elements_allocated * vt->element_size;
      void *temp = vector_alloc ( vt, new_size );

      if ( !temp )
        {
          vt->elements_allocated >>= 1;
          return 0;
        }

      if ( vt != temp )
        {
          vt = temp;
          *mem = CHUNCK_TO_MEM ( vt );
        }
    }

  vector_copy ( vt, data );

  vt->elements_used++;

  return 1;
}

void *
vector_pop ( void *mem )
{
  struct vector *vt = MEM_TO_CHUNK ( mem );

  vt->elements_used--;

  return ( void * ) ( ( char * ) mem + vt->elements_used * vt->element_size );
}

void
vector_clear ( void *mem )
{
  struct vector *vt = MEM_TO_CHUNK ( mem );

  vt->elements_used = 0;
}

size_t
vector_size ( void *mem )
{
  struct vector *vt = MEM_TO_CHUNK ( mem );

  return vt->elements_used;
}

void
vector_free ( void *mem )
{
  free ( MEM_TO_CHUNK ( mem ) );
}
