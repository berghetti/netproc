
#include <errno.h>
#include <string.h>
#include <time.h>

#include "m_error.h"

// multiply nanoseconds for this const convert nanoseconds to seconds
#define NSTOS 1E-9

inline static void
get_time ( struct timespec * );

double
start_timer ( void )
{
  struct timespec time;
  get_time ( &time );

  return ( double ) time.tv_sec + ( time.tv_nsec * NSTOS );
}

double
timer ( const float old_time )
{
  struct timespec new_time;
  get_time ( &new_time );

  double dif = ( new_time.tv_sec + ( new_time.tv_nsec * NSTOS ) ) - old_time;

  return dif;
}

inline static void
get_time ( struct timespec *buff_time )
{
  if ( clock_gettime ( CLOCK_MONOTONIC, buff_time ) == -1 )
    fatal_error ( "clock_gettime: %s", strerror ( errno ) );
}
