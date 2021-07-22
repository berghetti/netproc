
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

#ifndef HUMAN_READABLE_H
#define HUMAN_READABLE_H

#include <stdbool.h>  // bool type
#include <stdint.h>   // uint*_t type
#include <stdlib.h>   // size_t type

#include "config.h"

// mode human readable define sufix utilized
// RATE sufix/s
// TOTAL sufix
enum mode
{
  RATE = 1,
  TOTAL
};

void
define_sufix ( const struct config_op *co );

bool
human_readable ( char *buffer,
                 const size_t len_buff,
                 const uint64_t bytes,
                 int mode );

#endif  // HUMAN_READABLE_H
