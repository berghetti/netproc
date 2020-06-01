
/*
 *  Copyright (C) 2020 Mayco S. Berghetti
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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

#define BASE_IEC 1024  // default
#define BASE_SI 1000

#define LEN_ARR_SUFIX 5

// n = BASE_IEC ? 1 /1024 : 1/1000
#define INVERSE_BASE( n ) ( ( n ) == BASE_IEC ) ? 9.76562E-4 : 1E-3

extern int chosen_base;
extern const char *const *sufix_rate;
extern const char *const *sufix_total;

void
define_sufix ( void );

#endif  // SUFIX_H
