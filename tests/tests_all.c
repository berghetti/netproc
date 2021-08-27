
#include "unity.h"
// clang-format off

// include here test functions definition
void test_hashtable ( void );
void test_full_read ( void );
void test_queue ( void );
void test_str ( void );
void test_rate ( void );
void test_human_readable ( void );

// need to unity
void setUp ( void ) { /* set stuff up here */ }
void tearDown ( void ) { /*  clean stuff up here */ }
// clang-format on

int
main ( void )
{
  UNITY_BEGIN ();
  RUN_TEST ( test_hashtable );
  RUN_TEST ( test_full_read );
  RUN_TEST ( test_queue );
  RUN_TEST ( test_str );
  RUN_TEST ( test_rate );
  RUN_TEST ( test_human_readable );

  return UNITY_END ();
}
