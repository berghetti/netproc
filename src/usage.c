
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

#include "usage.h"
#include <stdio.h>

void
show_version ( void )
{
  puts ( PROG_NAME " - " PROG_VERSION );
}

void
usage ( void )
{
  show_version ();
  // clang-format off
  puts ( "Usage: " PROG_NAME " [options]\n"
         "\n"
         "Options:\n"
         "-u            tracks udp traffic, default is tcp\n"
         "-i <iface>    specifies an interface, default is all\n"
         "-B            view in bytes, default in bits\n"
         "-si           SI format display, with powers of 1000, default is IEC,"
                        " with powers of 1024\n"
         "-h            show this message\n"
         "-v            show version" );
  // clang-format on
}
