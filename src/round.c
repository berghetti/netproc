
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

#include <stdint.h>

// based in https://www.codeproject.com/Articles/58289/C-Round-Function

/* input never is negative here then
  we not verify number negative for more performance

 examples:
 input 1.1 - output 1
 input 1.4 - output 1
 input 1.5 - output 2
 input 1.9 - output 2 */
uint64_t
m_round ( double number )
{
  return ( uint64_t ) ( number + 0.5 );
}
