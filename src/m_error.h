
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

#ifndef ERROR_H
#define ERROR_H

#include <string.h>  // strerror
#include <errno.h>   // variable errno

#ifdef NDEBUG
#define ERROR_DEBUG( fmt, ... )
#else
#define ERROR_DEBUG( fmt, ... )                                        \
  do                                                                   \
    {                                                                  \
      debug_error ( "%s:%d - " fmt, __FILE__, __LINE__, __VA_ARGS__ ); \
    }                                                                  \
  while ( 0 )
#endif

// exibe mensagem na saida de erro padrão
// e sai com EXIT_FAILURE
void
fatal_error ( const char *msg, ... );

void
debug_error ( const char *msg, ... );

#endif  // ERROR_H
