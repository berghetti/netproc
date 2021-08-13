
#include "unity.h"
// clang-format off

// include here test functions definition
void test_hashtable ( void );
void test_full_read ( void );
void test_queue ( void );
void test_str( void );

// clang-format on

void
setUp ( void )
{ /* set stuff up here */
}
void
tearDown ( void )
{ /*  clean stuff up here */
}

int
main ( void )
{
  UNITY_BEGIN ();
  RUN_TEST ( test_hashtable );
  RUN_TEST ( test_full_read );
  RUN_TEST ( test_queue );
  RUN_TEST ( test_str );

  return UNITY_END ();
}
