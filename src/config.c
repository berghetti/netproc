
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

#include <stdlib.h>  // exit
#include <string.h>  // strncmp
#include <unistd.h>  // EXIT_*

#include "config.h"
#include "usage.h"
#include "m_error.h"

// default options
static struct config_op co = {
        .iface = NULL,  // all interfaces
        .proto = TCP | UDP,
        .view_si = false,
        .view_bytes = false,
        .view_conections = false,
        .translate_host = true,
        .translate_service = true,

        // internal control sinc of program, NOT REMOVE
        .tic_tac = 0,
};

struct config_op *
parse_options ( int argc, const char **argv )
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
                else if ( !strncasecmp ( ( *argv ), "tcp", sizeof ( "tcp" ) ) )
                  co.proto &= TCP;
                else if ( !strncasecmp ( *argv, "udp", sizeof ( "udp" ) ) )
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
