
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
#include <stdlib.h>  // exit
#include <string.h>  // strncmp
#include <unistd.h>  // EXIT_*

#include "config.h"
#include "conection.h"
#include "usage.h"

// default options
static struct config_op co = { .iface = NULL,  // all interfaces
                               .path_log = PROG_NAME_LOG,
                               .log = false,
                               .proto = TCP | UDP,
                               .color_scheme = 0,
                               .view_si = false,
                               .view_bytes = false,
                               .view_conections = false,
                               .translate_host = true,
                               .translate_service = true,
                               .verbose = false,
                               .running = 0 };

struct config_op *
parse_options ( int argc, char **argv )
{
  char *arg;
  char *prev;

  // skip arg 0
  argc--;
  argv++;

  arg = *argv++;
  while ( argc-- )
    {
      if ( !strcmp ( arg, "-B" ) || !strcmp ( arg, "--bytes" ) )
        {
          co.view_bytes = true;
        }
      else if ( !strcmp ( arg, "-c" ) )
        {
          co.view_conections = true;
        }
      else if ( !strcmp ( arg, "--color" ) )
        {
          prev = arg;
          arg = *argv;

          int value;
          if ( !arg || !( value = atoi ( arg ) ) )
            {
              fprintf ( stderr,
                        "'%s' argument requires a valid color scheme number\n",
                        prev );
              goto FAIL;
            }

          // 0, 1 or 2
          value &= 0x3;
          co.color_scheme = value - ( value != 0 );

          argc--;
          argv++;
        }
      else if ( !strcmp ( arg, "-f" ) || !strcmp ( arg, "--file" ) )
        {
          co.log = true;
          arg = *argv;
          if ( arg && *arg != '-' && !!strcmp ( arg, "--" ) )
            {
              co.path_log = arg;
              argv++;
              argc--;
            }
        }
      else if ( !strcmp ( arg, "-h" ) || !strcmp ( arg, "--help" ) )
        {
          usage ();
          exit ( EXIT_SUCCESS );
        }
      else if ( !strcmp ( arg, "-i" ) || !strcmp ( arg, "--interface" ) )
        {
          prev = arg;
          arg = *argv;
          if ( !arg || ( *arg == '-' || !strncmp ( arg, "--", 2 ) ) )
            {
              fprintf (
                      stderr, "Argument '%s' requere interface name\n", prev );
              goto FAIL;
            }

          co.iface = arg;
          argc--;
          argv++;
        }
      else if ( !strcmp ( arg, "-n" ) )
        {
          // implict
          co.view_conections = true;

          co.translate_host = false;
          co.translate_service = false;
        }
      else if ( !strcmp ( arg, "-nh" ) )
        {
          // implict
          co.view_conections = true;

          // no translate only host
          co.translate_host = false;
        }
      else if ( !strcmp ( arg, "-np" ) )
        {
          // implict
          co.view_conections = true;

          // no translate only service (port)
          co.translate_service = false;
        }
      else if ( !strcmp ( arg, "-p" ) || !strcmp ( arg, "--protocol" ) )
        {
          prev = arg;
          arg = *argv;

          if ( !arg || ( *arg == '-' || !strncmp ( arg, "--", 2 ) ) )
            {
              fprintf ( stderr, "Argument '%s' requere udp or tcp\n", prev );
              goto FAIL;
            }
          else if ( !strcasecmp ( arg, "tcp" ) )
            co.proto &= TCP;
          else if ( !strcasecmp ( arg, "udp" ) )
            co.proto &= UDP;
          else
            {
              fprintf ( stderr, "invalid protocol in argument '%s'\n", prev );
              goto FAIL;
            }

          argc--;
          argv++;
        }
      else if ( !strcmp ( arg, "--si" ) )
        {
          co.view_si = true;
        }
      else if ( !strcmp ( arg, "-v" ) || !strcmp ( arg, "--verbose" ) )
        {
          co.verbose = true;
        }
      else if ( !strcmp ( arg, "-V" ) || !strcmp ( arg, "--version" ) )
        {
          show_version ();
          exit ( EXIT_SUCCESS );
        }

      else
        {
          fprintf ( stderr, "Invalid argument '%s'\n", arg );
          goto FAIL;
        }

      arg = *argv++;
    }

  return &co;

FAIL:
  fprintf ( stderr, "See " PROG_NAME " --help\n" );
  exit ( EXIT_FAILURE );
}
