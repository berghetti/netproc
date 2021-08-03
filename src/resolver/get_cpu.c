
/*
 *  Copyright (C) 2021 Mayco S. Berghetti
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

#include <unistd.h>

// return tot cpu available on system or 0
int
get_count_cpu ( void )
{
  int ret = 0;

// GNU extension
#ifdef _SC_NPROCESSORS_ONLN
  ret = ( int ) sysconf ( _SC_NPROCESSORS_ONLN );
  if ( ret < 0 )
    ret = 0;
#endif

  return ret;
}
