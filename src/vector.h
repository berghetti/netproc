
#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

/*
 create new vector
 @param nmeb, number of member initial in vector, if 0 default is 16
 @param size, size of one member in array

 @return memory to user or null
*/
void *
vector_new ( size_t nmeb, size_t size_member );

/*
 copy user data into vector
 @oaram mem, pointer returned from vector_new
 @param data, pointer to user data to push in vector

 @return 1 if success or 0 in error
*/
int
vector_push_ ( void **mem, void *data );

#define vector_push( mem, data ) vector_push_ ( ( void ** ) ( &mem ), ( data ) )

/*
  remove last element from vector
  @oaram mem, pointer returned from vector_new

  @return number of elements in vector
*/
int
vector_pop ( void *mem );

/*
  remove all elements in vector
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
  free dinamyc memory allocated to vector
  @oaram mem, pointer returned from vector_new
*/
void
vector_free ( void *mem );

#endif  // VECTOR_H
