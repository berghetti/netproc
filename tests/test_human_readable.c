
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "unity.h"
#include "config.h"
#include "human_readable.h"

#pragma GCC diagnostic ignored "-Wunused-value"

// #define DPRINT 1

#ifdef DPRINT
#define PRINT printf
#else
#define PRINT
#endif

#define ARRAY_SIZE( ar ) ( sizeof ( ar ) / sizeof ( ar[0] ) )

static char buff[14];

void
test_Byte_per_second_si ( void )
{
  int values[] = { 0, 500, 1024, 2048, 5000, 9999, 10485760 };
  char *expected[] = { "0",          "500 B/s",    "1.00 KiB/s", "2.00 KiB/s",
                       "4.88 KiB/s", "9.76 KiB/s", "10.00 MiB/s" };

  define_sufix ( false, true );

  for ( size_t i = 0; i < ARRAY_SIZE ( values ); i++ )
    {
      TEST_ASSERT_TRUE (
              human_readable ( buff, sizeof ( buff ), values[i], RATE ) );
      PRINT ( "%s\n", buff );
      TEST_ASSERT_EQUAL_STRING ( expected[i], buff );
    }
}

void
test_bit_per_second_si ( void )
{
  int values[] = { 0, 1015, 1024, 2048, 6000, 9999, 10485760 };
  char *expected[] = { "0",          "1015 b/s",   "1.00 Kib/s", "2.00 Kib/s",
                       "5.86 Kib/s", "9.76 Kib/s", "10.00 Mib/s" };

  define_sufix ( false, false );

  for ( size_t i = 0; i < ARRAY_SIZE ( values ); i++ )
    {
      TEST_ASSERT_TRUE (
              human_readable ( buff, sizeof ( buff ), values[i], RATE ) );
      PRINT ( "%s\n", buff );
      TEST_ASSERT_EQUAL_STRING ( expected[i], buff );
    }
}

void
test_bit_per_second_iec ( void )
{
  int values[] = { 0, 999, 1015, 1024, 6000, 9999, 10485760 };
  char *expected[] = { "0",         "999 b/s",    "1.01 Kb/s", "1.02 Kb/s",
                       "6.00 Kb/s", "10.00 Kb/s", "10.48 Mb/s" };

  define_sufix ( true, false );

  for ( size_t i = 0; i < ARRAY_SIZE ( values ); i++ )
    {
      TEST_ASSERT_TRUE (
              human_readable ( buff, sizeof ( buff ), values[i], RATE ) );
      PRINT ( "%s\n", buff );
      TEST_ASSERT_EQUAL_STRING ( expected[i], buff );
    }
}

void
test_human_readable ( void )
{
  test_Byte_per_second_si ();

  test_bit_per_second_si ();

  test_bit_per_second_iec ();
}
