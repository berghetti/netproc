
// #include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "unity.h"
#include "vector.h"

void
test_vector_int ( void )
{
#define COUNT 0xF

  int *vi = vector_new ( sizeof ( int ) );

  TEST_ASSERT_TRUE ( vi );

  for ( size_t i = 0; i < COUNT; ++i )
    {
      TEST_ASSERT_TRUE ( vector_push ( vi, &i ) );
    }

  TEST_ASSERT_EQUAL_INT ( COUNT, vector_size ( vi ) );

  for ( size_t i = 0; i < COUNT; i++ )
    {
      int *d = vector_pop ( vi );
      TEST_ASSERT_EQUAL_INT ( COUNT - i - 1, *d );
      TEST_ASSERT_EQUAL_INT ( i, vi[i] );
    }

  vector_free ( vi );
}

struct mydata
{
  int a;
  int b;
};

void
test_vector_my_data ( void )
{
  struct mydata data1 = { .a = 1, .b = 2 }, data2 = { .a = 5, .b = 10 };

  struct mydata *vt_data = vector_new ( sizeof *vt_data );

  TEST_ASSERT_TRUE ( vt_data );

  TEST_ASSERT_TRUE ( vector_push ( vt_data, &data1 ) );
  TEST_ASSERT_TRUE ( vector_push ( vt_data, &data2 ) );

  TEST_ASSERT_EQUAL_INT ( 2, vector_size ( vt_data ) );

  TEST_ASSERT_EQUAL_INT ( 1, vt_data[0].a );
  TEST_ASSERT_EQUAL_INT ( 2, vt_data[0].b );
  TEST_ASSERT_EQUAL_INT ( 5, vt_data[1].a );
  TEST_ASSERT_EQUAL_INT ( 10, vt_data[1].b );

  struct mydata *res = vector_pop ( vt_data );

  TEST_ASSERT_EQUAL_INT ( 5, res->a );
  TEST_ASSERT_EQUAL_INT ( 10, res->b );

  TEST_ASSERT_EQUAL_INT ( 1, vector_size ( vt_data ) );

  // printf("%p\n%p\n", &data2, res );
  //
  // printf("%d\n%d\n", res->b, res->a );

  vector_free ( vt_data );
}

void
test_heap ( void )
{
  int **v = vector_new ( sizeof ( int * ) );

  TEST_ASSERT_NOT_NULL ( v );

  int size = 5;

  for ( int i = 0; i < size; i++ )
    {
      int *d = malloc ( sizeof ( int ) );
      *d = i;
      vector_push ( v, &d );
    }

  for ( int i = 0; i < size; i++ )
    {
      int **d = vector_pop ( v );

      // printf("%d\n", **d);
      TEST_ASSERT_EQUAL_INT ( size - i - 1, **d );
      free ( *d );
    }

  vector_free ( v );
}

void
test_vector ( void )
{
  test_vector_int ();
  test_vector_my_data ();
  test_heap ();
}
