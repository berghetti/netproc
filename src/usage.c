
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
  fputs ( "Usage: " PROG_NAME " [options]\n"
         "\n"
         "Options:\n"
         " -B            view in bytes, default in bits\n"
         " -c            visualization each active connection of the process\n"
         " -h            show this message\n"
         " -i <iface>    specifies an interface, default is all (except interface with network 127.0.0.0/8)\n"
         " -n            not translate host and service, implicit '-c',\n"
         "               try '-nh' to no translate only host or '-np' to not translate only service\n"
         " -p tcp | udp  specifies only a protocol, the default is tcp and udp"
         " -si           SI format display, with powers of 1000, default is IEC, with powers of 1024\n"
         " -v            show version\n"
         "\n"
         "when running press:\n"
         " arrow keys    scroll\n"
         " s             change column-based sort\n"
         " q              exit"
         , stderr);
  // clang-format on
}
