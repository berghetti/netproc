
/*
 *  Copyright (C) 2021-2022 Mayco S. Berghetti
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

#ifndef MACRO_UTIL_H
#define MACRO_UTIL_H

#define MAX( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )

#define ARRAY_SIZE( x ) ( sizeof ( x ) / sizeof ( x[0] ) )

#ifdef __GNUC__
#define UNUSED( x ) __attribute__ ( ( __unused__ ) ) x
#define fallthrough __attribute__ ( ( __fallthrough__ ) )
#else
#define UNUSED( x )
#define fallthrough
#endif

#endif  // MACRO_UTIL_H
