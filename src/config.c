
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
#include "m_error.h"

// default options
static struct config_op co = {
        .iface = NULL,  // all interfaces
        .path_log = PROG_NAME_LOG,
        .log = false,
        .proto = TCP | UDP,
        .view_si = false,
        .view_bytes = false,
        .view_conections = false,
        .translate_host = true,
        .translate_service = true,

        // internal control sinc of program, NOT REMOVE
        .tic_tac = 0,
        .running = 0,
};

struct config_op *
parse_options2 ( int argc, char **argv )
{
  char *arg;

  // skip arg 0
  argc--;
  argv++;

  arg = *argv++;
  while ( argc-- )
    {
      fprintf ( stderr, "%s\n", arg );

      if ( !strcmp ( arg, "-B" ) || !strcmp ( arg, "--bytes" ) )
        co.view_bytes = true;
      else if ( !strcmp ( arg, "-c" ) )
        co.view_conections = true;
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
          arg = *argv;
          if ( !arg || ( *arg == '-' || !strncmp ( arg, "--", 2 ) ) )
            {
              error ( "Argument '-i, --interface' requere interface name" );
              usage ();
              exit ( EXIT_FAILURE );
            }

          co.iface = arg;
          argc--;
          argv++;
        }
      else if ( !strcmp ( arg, "-n" ) )
        {
          fprintf ( stderr, "%s\n", "traduzindo -n" );
          // implict
          co.view_conections = true;

          co.translate_host = false;
          co.translate_service = false;
        }
      else if ( !strcmp ( arg, "-nh" ) )
        {
          fprintf ( stderr, "%s\n", "traduzindo -nh" );
          // implict
          co.view_conections = true;

          // no translate only service (port)
          co.translate_host = false;
        }
      else if ( !strcmp ( arg, "-np" ) )
        {
          fprintf ( stderr, "%s\n", "traduzindo -np" );
          // implict
          co.view_conections = true;

          // no translate only service (port)
          co.translate_service = false;
        }
      else
        {
          error ( "Invalid argument '%s'", arg );
          usage ();
          exit ( EXIT_FAILURE );
        }

      arg = *argv++;
    }

  return &co;
}

struct config_op *
parse_options ( int argc, char **argv )
{
  // parse options
  while ( --argc )
    {
      if ( *++argv && **argv == '-' )
        {
          switch ( *( *argv + 1 ) )
            {
              case 'B':
                co.view_bytes = true;
                break;
              case 'c':
                co.view_conections = true;
                break;
              case 'f':
                co.log = true;

                // optional name file
                if ( *( argv + 1 ) && *( *( argv + 1 ) ) != '-' )
                  {
                    co.path_log = ( char * ) *++argv;
                    argc--;
                  }

                break;
              case 'h':
                usage ();
                exit ( EXIT_SUCCESS );
                break;
              case 'i':
                if ( !( *++argv ) || **argv == '-' )
                  {
                    error ( "Argument '-i' requere interface name" );
                    usage ();
                    exit ( EXIT_FAILURE );
                  }

                co.iface = ( char * ) *argv;
                argc--;
                break;
              case 'n':
                // view conections implict
                co.view_conections = true;

                if ( *( *argv + 2 ) && *( *argv + 2 ) == 'h' )
                  {
                    co.translate_host = false;
                    break;
                  }
                else if ( *( *argv + 2 ) && *( *argv + 2 ) == 'p' )
                  {
                    co.translate_service = false;
                    break;
                  }
                else if ( ( *( *argv + 2 ) ) )
                  goto FAIL;

                co.translate_host = false;
                break;
              case 'p':

                if ( !( *++argv ) || **argv == '-' )
                  {
                    error ( "Argument '-p' requere protocol tcp or udp" );
                    usage ();
                    exit ( EXIT_FAILURE );
                  }
                else if ( !strncasecmp ( *argv, "tcp", strlen ( "tcp" ) ) )
                  co.proto &= TCP;
                else if ( !strncasecmp ( *argv, "udp", strlen ( "udp" ) ) )
                  co.proto &= UDP;
                else
                  {
                    error ( "invalid protocol in argument '-p'" );
                    usage ();
                    exit ( EXIT_FAILURE );
                  }

                argc--;
                break;
              case 's':
                if ( *( *argv + 2 ) && *( *argv + 2 ) == 'i' )
                  {
                    co.view_si = true;
                    break;
                  }
                goto FAIL;
              case 'v':
                show_version ();
                exit ( EXIT_SUCCESS );
              default:
                goto FAIL;
            }
        }
      else
        goto FAIL;
    }

  return &co;

FAIL:
  error ( "Invalid argument '%s'", *argv );
  usage ();
  exit ( EXIT_FAILURE );
}
