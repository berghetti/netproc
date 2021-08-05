
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

#define _GNU_SOURCE  // for asprintf
#include <errno.h>   // variable errno
#include <stdarg.h>  // va_*
#include <stdio.h>
#include <stdlib.h>
#include <term.h>    // tputs
#include <signal.h>  // raise
#include <unistd.h>

#include "m_error.h"

#define DEBUG "[DEBUG] "
#define ERROR "[ERROR] "
#define FATAL "[FATAL] "

static void
print ( const char *restrict tag, const char *restrict fmt, va_list args )
{
  fprintf ( stderr, "%s", tag );
  vfprintf ( stderr, fmt, args );
  fprintf ( stderr, "\n" );
}

void
fatal_error ( const char *fmt, ... )
{
  va_list args;

  va_start ( args, fmt );
  print ( FATAL, fmt, args );
  va_end ( args );
}

void
debug_error ( const char *fmt, ... )
{
  va_list args;

  va_start ( args, fmt );
  print ( DEBUG, fmt, args );
  va_end ( args );
}
