
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
#include <string.h>
#include <time.h>  // struct timespec

int
start_timer ( struct timespec *ts )
{
  return ( clock_gettime ( CLOCK_MONOTONIC, ts ) != -1 );
}

uint64_t
diff_timer ( struct timespec *old_time )
{
  struct timespec new_time;
  clock_gettime ( CLOCK_MONOTONIC, &new_time );

  new_time.tv_sec -= old_time->tv_sec;
  new_time.tv_nsec -= old_time->tv_nsec;

  // convert to milliseconds
  return new_time.tv_sec * 1000 + new_time.tv_nsec / 1E6;
}

// hh:mm:ss
#define LEN_BUFF_CLOCK 14

char *
sec2clock ( uint64_t milliseconds )
{
  static char clock[LEN_BUFF_CLOCK];

  uint32_t secs = milliseconds / 1000U;

  uint32_t hours, seconds, minutes;

  seconds = secs % 60UL;
  secs /= 60UL;
  minutes = secs % 60UL;
  hours = secs / 60UL;

  snprintf ( clock,
             sizeof clock,
             "%02d:%02d:%02d",
             hours,      // hour
             minutes,    // minute
             seconds );  // second

  return clock;
}
