// gcc -o tick_tack tick_tack.c ../src/timer.c -Wall -Wextra && ./tick_tack

// this is a stupid test done to check for a bug,
// run this will stress the cpu

#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "../src/timer.h"

// in milliseconds
#define ONE_SEC 1000

void
tick_tack ( uint64_t running )
{
  static int tick_tack = 0;

  printf ( "%s - %s\n",
           ( tick_tack ) ? "tack" : "tick",
           msec2clock ( running ) );

  tick_tack = !tick_tack;
}

int
main ( void )
{
  uint64_t cur_time = get_time ();
  uint64_t running = 0;

  tick_tack ( running );
  while ( 1 )
    {
      uint64_t new_time = get_time ();

      if ( new_time < cur_time )
        continue;

      uint32_t diff_time = new_time - cur_time;

      if ( diff_time >= ONE_SEC )
        {
          running += diff_time;
          diff_time -= ONE_SEC;
          cur_time = new_time + diff_time;

          tick_tack ( running );
        }
    }

  return 0;
}
