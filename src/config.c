
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
#include <stdbool.h>
#include <stdlib.h>  // exit
#include <string.h>  // strncmp
#include <unistd.h>  // EXIT_*

#include "config.h"
#include "conection.h"
#include "usage.h"
#include "macro_util.h"

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

static void
fatal_config ( const char *msg )
{
  fprintf ( stderr,
            "%s\n"
            "See " PROG_NAME " --help\n",
            msg );
  exit ( EXIT_FAILURE );
}

static void
view_bytes ( UNUSED ( char *arg ) )
{
  co.view_bytes = true;
}

static void
view_conections ( UNUSED ( char *arg ) )
{
  co.view_conections = true;
}

static void
color_scheme ( char *arg )
{
  int value;
  if ( !arg || !( value = atoi ( arg ) ) )
    fatal_config ( "'--color' argument requires a valid color scheme number" );

  // 0, 1 or 2
  value &= 0x3;
  co.color_scheme = value - ( value != 0 );
}

static void
log_file ( char *arg )
{
  co.log = true;

  if ( arg )
    co.path_log = arg;
}

struct cmd
{
  char *cur_opt;
  char *long_opt;
  void ( *func ) ( char * );
  int req_arg;
};

// cmd.req_arg values
#define NO_ARG 0
#define REQ_ARG 1
#define OPT_ARG 2

static inline int
check_cmd ( char *arg, const struct cmd *cmd )
{
  return !strcmp ( arg, cmd->cur_opt ) || !strcmp ( arg, cmd->long_opt );
}

struct config_op *
parse_options ( int argc, char **argv )
{
  static const struct cmd cmd[] = {
          { "-B", "--bytes", view_bytes, NO_ARG },
          { "-c", "", view_conections, NO_ARG },
          { "", "--color", color_scheme, REQ_ARG },
          { "-f", "--file", log_file, OPT_ARG },
  };

  // skip arg 0
  argc--;
  argv++;

  while ( argc-- )
    {
      int check = 0;

      for ( unsigned int i = 0; i < ARRAY_SIZE ( cmd ); i++ )
        {
          check = check_cmd ( *argv, &cmd[i] );
          if ( check )
            {
              char *arg = NULL;

              switch ( cmd[i].req_arg )
                {
                  case REQ_ARG:
                      argv++;
                      argc--;
                      arg = *argv;
                      break;
                  case OPT_ARG:
                    argv++;
                    if ( *argv == NULL || **argv == '-' )
                      {
                        argv--;
                        break;
                      }

                    argc--;
                    arg = *argv;
                }

              cmd[i].func ( arg );
              break;
            }
        }

      if ( !check )
        {
          fprintf ( stderr,
                    "Invalid Argument '%s'\n"
                    "See " PROG_NAME " --help\n",
                    *argv );
          exit ( EXIT_FAILURE );
        }

      argv++;

      //   if ( !strcmp ( arg, "-B" ) || !strcmp ( arg, "--bytes" ) )
      //     {
      //       co.view_bytes = true;
      //     }
      //   else if ( !strcmp ( arg, "-c" ) )
      //     {
      //       co.view_conections = true;
      //     }
      //   else if ( !strcmp ( arg, "--color" ) )
      //     {
      //       prev = arg;
      //       arg = *argv;
      //
      //       int value;
      //       if ( !arg || !( value = atoi ( arg ) ) )
      //         {
      //           fprintf ( stderr,
      //                     "'%s' argument requires a valid color scheme
      //                     number\n", prev );
      //           goto FAIL;
      //         }
      //
      //       // 0, 1 or 2
      //       value &= 0x3;
      //       co.color_scheme = value - ( value != 0 );
      //
      //       argc--;
      //       argv++;
      //     }
      //   else if ( !strcmp ( arg, "-f" ) || !strcmp ( arg, "--file" ) )
      //     {
      //       co.log = true;
      //       arg = *argv;
      //       if ( arg && *arg != '-' && !!strcmp ( arg, "--" ) )
      //         {
      //           co.path_log = arg;
      //           argv++;
      //           argc--;
      //         }
      //     }
      //   else if ( !strcmp ( arg, "-h" ) || !strcmp ( arg, "--help" ) )
      //     {
      //       usage ();
      //       exit ( EXIT_SUCCESS );
      //     }
      //   else if ( !strcmp ( arg, "-i" ) || !strcmp ( arg, "--interface" ) )
      //     {
      //       prev = arg;
      //       arg = *argv;
      //       if ( !arg || ( *arg == '-' || !strncmp ( arg, "--", 2 ) ) )
      //         {
      //           fprintf (
      //                   stderr, "Argument '%s' requere interface name\n",
      //                   prev );
      //           goto FAIL;
      //         }
      //
      //       co.iface = arg;
      //       argc--;
      //       argv++;
      //     }
      //   else if ( !strcmp ( arg, "-n" ) )
      //     {
      //       // implict
      //       co.view_conections = true;
      //
      //       co.translate_host = false;
      //       co.translate_service = false;
      //     }
      //   else if ( !strcmp ( arg, "-nh" ) )
      //     {
      //       // implict
      //       co.view_conections = true;
      //
      //       // no translate only host
      //       co.translate_host = false;
      //     }
      //   else if ( !strcmp ( arg, "-np" ) )
      //     {
      //       // implict
      //       co.view_conections = true;
      //
      //       // no translate only service (port)
      //       co.translate_service = false;
      //     }
      //   else if ( !strcmp ( arg, "-p" ) || !strcmp ( arg, "--protocol" ) )
      //     {
      //       prev = arg;
      //       arg = *argv;
      //
      //       if ( !arg || ( *arg == '-' || !strncmp ( arg, "--", 2 ) ) )
      //         {
      //           fprintf ( stderr, "Argument '%s' requere udp or tcp\n", prev
      //           ); goto FAIL;
      //         }
      //       else if ( !strcasecmp ( arg, "tcp" ) )
      //         co.proto &= TCP;
      //       else if ( !strcasecmp ( arg, "udp" ) )
      //         co.proto &= UDP;
      //       else
      //         {
      //           fprintf ( stderr, "invalid protocol in argument '%s'\n", prev
      //           ); goto FAIL;
      //         }
      //
      //       argc--;
      //       argv++;
      //     }
      //   else if ( !strcmp ( arg, "--si" ) )
      //     {
      //       co.view_si = true;
      //     }
      //   else if ( !strcmp ( arg, "-v" ) || !strcmp ( arg, "--verbose" ) )
      //     {
      //       co.verbose = true;
      //     }
      //   else if ( !strcmp ( arg, "-V" ) || !strcmp ( arg, "--version" ) )
      //     {
      //       show_version ();
      //       exit ( EXIT_SUCCESS );
      //     }
      //
      //   else
      //     {
      //       fprintf ( stderr, "Invalid argument '%s'\n", arg );
      //       goto FAIL;
      //     }
      //
      // arg = *argv++;
    }

  return &co;
}
