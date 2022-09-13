
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

#include <stdio.h>
#include "config.h"

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
         " -B, --bytes             view in bytes, default in bits\n"
         " -c                      visualization each active connection of the process\n"
         " --color 1|2|3           color scheme, 1 is default\n"
         " -f, --file \"filename\"   save statistics in file, filename is optional,\n"
         "                         default is '" PROG_NAME_LOG "'\n"
         " -h, --help              show this message\n"
         " -i, --interface iface   specifies an interface, default is all\n"
         "                         (except interface with network 127.0.0.0/8)\n"
         " -n                      numeric host and service, implicit '-c', try '-nh' to no\n"
         "                         translate only host or '-np' to not translate only service\n"
         " -p, --protocol tcp|udp  specifies a protocol, the default is tcp and udp\n"
         " --si                    show SI format, with powers of 10, default is IEC,\n"
         "                         with powers of 2\n"
         " -v, --verbose           verbose mode, also show process without traffic\n"
         " -V, --version           show version\n"
         "\n"
         "when running press:\n"
         " arrow keys    scroll\n"
         " s             change column-based sort\n"
         " q             exit\n"
         , stderr);
  // clang-format on
}
