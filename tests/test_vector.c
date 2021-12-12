
#include <stddef.h>

#include "unity.h"
#include "vector.h"

void
test_vector_int ( void )
{
#define COUNT 0xFFFF

  int *vi = vector_new ( 0, sizeof ( int ) );

  TEST_ASSERT_TRUE ( vi );

  for ( size_t i = 0; i < COUNT; ++i )
    {
      TEST_ASSERT_TRUE ( vector_push ( vi, &i ) );
    }

  TEST_ASSERT_EQUAL_INT ( COUNT, vector_size ( vi ) );

  for ( size_t i = 0; i < vector_size ( vi ); i++ )
    {
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

  struct mydata *vt_data = vector_new ( 0, sizeof *vt_data );

  TEST_ASSERT_TRUE ( vt_data );

  TEST_ASSERT_TRUE ( vector_push ( vt_data, &data1 ) );
  TEST_ASSERT_TRUE ( vector_push ( vt_data, &data2 ) );

  TEST_ASSERT_EQUAL_INT ( 2, vector_size ( vt_data ) );

  TEST_ASSERT_EQUAL_INT ( 1, vt_data[0].a );
  TEST_ASSERT_EQUAL_INT ( 2, vt_data[0].b );
  TEST_ASSERT_EQUAL_INT ( 5, vt_data[1].a );
  TEST_ASSERT_EQUAL_INT ( 10, vt_data[1].b );

  vector_free ( vt_data );
}

void
test_vector ( void )
{
  test_vector_int ();
  test_vector_my_data ();
}
