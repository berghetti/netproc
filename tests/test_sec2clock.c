
#include "timer.h"
#include "unity.h"

#include "cycle_counting.h"

struct test_set
{
  uint64_t ml;
  char *expected;
};

static struct test_set test[] = {
  { 0, "00:00:00" },         { 1000, "00:00:01" },
  { 53646154, "14:54:06" },  { 172800000, "48:00:00" },
  { 172810000, "48:00:10" }, { 1800752000, "500:12:32" }
};

void
test_performance ( void )
{
  counter_T cnt;

  cnt = BEGIN_TSC ();
  char *p = msec2clock ( 1800752000 );
  cnt = END_TSC ( cnt );

  printf ( "%s\ncycles = %lu\n", p, cnt );
}

#define ARRAY_SIZE( x ) ( sizeof ( x ) / sizeof ( x[0] ) )

void
test_sec2clock ( void )
{
  for ( uint32_t i = 0; i < ARRAY_SIZE ( test ); i++ )
    {
      TEST_ASSERT_EQUAL_STRING ( test[i].expected, msec2clock ( test[i].ml ) );
    }

  // test_performance();
}
