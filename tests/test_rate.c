
#include <stdint.h>

#include "unity.h"
#include "processes.h"
#include "config.h"

#include "rate.h"

void
exec ( struct processes *processes )
{
  process_t *proc = *( processes->proc );

  struct config_op co = { .view_bytes = 1 };

  uint64_t expected_rx = 0, expected_tx = 0;

  // simulate 5 seconds of test
  int times = 5;
  while ( times-- )
    {
      for ( int i = 0; i < SAMPLE_SPACE_SIZE; i++ )
        {
          // first time is needed increase of value expected
          if ( times == 4 )
            {
              expected_rx += 1000;  // exact value to avoid rounding
              expected_tx += 500;
            }

          // max value after first time must be always 5000 / SAMPLE_SPACE_SIZE
          rate_add_rx ( &proc->net_stat, 1000 );
          rate_add_tx ( &proc->net_stat, 500 );

          rate_calc ( processes, &co );

          TEST_ASSERT_EQUAL_INT ( expected_rx / SAMPLE_SPACE_SIZE,
                                  proc->net_stat.avg_Bps_rx );
          TEST_ASSERT_EQUAL_INT ( expected_tx / SAMPLE_SPACE_SIZE,
                                  proc->net_stat.avg_Bps_tx );

          // rotate e clean slot
          rate_update ( processes, &co );
        }
    }
}

void
test_rate ( void )
{
  process_t *proc = calloc ( 1, sizeof *proc );
  process_t *pp_procs[] = { proc, NULL };
  struct processes processes = { .proc = pp_procs, .total = 1 };

  exec ( &processes );

  free ( proc );
}
