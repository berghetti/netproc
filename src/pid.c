
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

#include <stdio.h>  // FILE*
#include <stdint.h>

// this can even represent an unsigned integer (4294967295)
#define MAX_DIGITS_UINT32 10

#define PATH_MAX_PID "/proc/sys/kernel/pid_max"

uint32_t
get_max_digits_pid ( void )
{
  FILE *file = fopen ( PATH_MAX_PID, "r" );
  if ( !file )
    return MAX_DIGITS_UINT32;

  char max_pid[MAX_DIGITS_UINT32 + 1];
  int match = fscanf ( file, "%10s", max_pid );
  fclose ( file );

  if ( 1 != match )
    return MAX_DIGITS_UINT32;

  char *p = max_pid;
  while ( *p )
    p++;

  return p - max_pid;
}
