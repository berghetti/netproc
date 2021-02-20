
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

#ifndef SUFIX_H
#define SUFIX_H

#include "config.h"

#define BASE_IEC 1024  // default
#define BASE_SI 1000

// b, KB, MB, GB, TB, PB
#define TOT_ELEMENTS_SUFIX 6

// n = BASE_IEC ? 1 /1024 : 1/1000
#define INVERSE_BASE( n ) ( ( n ) == BASE_IEC ) ? 9.76562E-4 : 1E-3

extern int chosen_base;
extern const char *const *sufix_rate;
extern const char *const *sufix_total;

void
define_sufix ( const struct config_op *co );

#endif  // SUFIX_H
