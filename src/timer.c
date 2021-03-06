
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
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "m_error.h"

// multiply nanoseconds for this const convert nanoseconds TO seconds
#define NSTOS 1E-9

// hh:mm:ss
#define LEN_BUFF_CLOCK 14

static inline bool
get_time ( struct timespec *buff_time )
{
  if ( clock_gettime ( CLOCK_MONOTONIC, buff_time ) == -1 )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return false;
    }

  return true;
}

double
start_timer ( void )
{
  struct timespec time;
  if ( !get_time ( &time ) )
    return -1;

  return ( double ) time.tv_sec + ( time.tv_nsec * NSTOS );
}

double
timer ( const float old_time )
{
  struct timespec new_time;
  if ( !get_time ( &new_time ) )
    return -1;

  return ( new_time.tv_sec + ( new_time.tv_nsec * NSTOS ) ) - old_time;
}

char *
sec2clock ( uint64_t secs )
{
  static char clock[LEN_BUFF_CLOCK];

  snprintf ( clock,
             LEN_BUFF_CLOCK,
             "%02d:%02d:%02d",
             ( int ) secs / 3600,             // hour
             ( int ) ( secs % 3600 ) / 60,    // minute
             ( int ) ( secs % 3600 ) % 60 );  // second

  return clock;
}
