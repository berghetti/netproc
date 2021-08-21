
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "m_error.h"

static inline int
get_time ( struct timespec *buff_time )
{
  if ( clock_gettime ( CLOCK_MONOTONIC, buff_time ) == -1 )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return 0;
    }

  return 1;
}

#define NSTOMS 1E-6  // nanoseconds to milliseconds
#define STOMS 1000   // seconds to milliseconds

long
start_timer ( void )
{
  struct timespec time;
  if ( !get_time ( &time ) )
    return -1;

  return ( time.tv_sec * STOMS ) + ( time.tv_nsec * NSTOMS );
}

long
timer ( const long old_time )
{
  struct timespec new_time;
  if ( !get_time ( &new_time ) )
    return -1;

  return ( ( ( new_time.tv_sec * STOMS ) + ( new_time.tv_nsec * NSTOMS ) ) ) -
         old_time;
}

int
start_timer2 ( struct timespec *ts )
{
  return ( clock_gettime ( CLOCK_MONOTONIC, ts ) != -1 );
}

uint64_t
diff_timer ( struct timespec *old_time )
{
  struct timespec new_time;
  if ( clock_gettime ( CLOCK_MONOTONIC, &new_time ) == -1 )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return -1;
    }

  new_time.tv_sec -= old_time->tv_sec;
  new_time.tv_nsec -= old_time->tv_nsec;

  return new_time.tv_sec * 1000 + new_time.tv_nsec / 1E6;
}

// hh:mm:ss
#define LEN_BUFF_CLOCK 14

char *
sec2clock ( uint64_t milliseconds )
{
  static char clock[LEN_BUFF_CLOCK];

  uint32_t secs = milliseconds / 1000;

  snprintf ( clock,
             LEN_BUFF_CLOCK,
             "%02d:%02d:%02d",
             ( int ) secs / 3600,             // hour
             ( int ) ( secs % 3600 ) / 60,    // minute
             ( int ) ( secs % 3600 ) % 60 );  // second

  return clock;
}
