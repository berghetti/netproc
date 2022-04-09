
#include "unity.h"
// clang-format off

// include here test functions definition
void test_hashtable ( void );
void test_full_read ( void );
void test_queue ( void );
void test_str ( void );
void test_rate ( void );
void test_human_readable ( void );
void test_packet ( void );
void test_vector ( void );
void test_sec2clock ( void );
void test_ht_conn( void );

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
  RUN_TEST ( test_packet );
  RUN_TEST ( test_vector );
  RUN_TEST ( test_sec2clock );
  RUN_TEST ( test_ht_conn );

  return UNITY_END ();
}
