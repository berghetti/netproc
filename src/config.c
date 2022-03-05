
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
#include "connection.h"
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
view_bytes ( UNUSED char *arg )
{
  co.view_bytes = true;
}
static void
view_conections ( UNUSED char *arg )
{
  co.view_conections = true;
}

static void
color_scheme ( char *arg )
{
  int value;
  if ( !arg || !( value = atoi ( arg ) ) )
    fatal_config ( "Argument '--color' requires a valid color scheme number" );

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

static void
show_help ( UNUSED char *arg )
{
  usage ();
  exit ( EXIT_SUCCESS );
}

static void
iface ( char *arg )
{
  if ( !arg )
    fatal_config ( "Argument '-i' requere interface name" );

  co.iface = arg;
}

static void
show_numeric_host ( UNUSED char *arg )
{
  co.view_conections = true;
  co.translate_host = false;
}
static void
show_numeric_port ( UNUSED char *arg )
{
  co.view_conections = true;
  co.translate_service = false;
}

static void
show_numeric ( UNUSED char *arg )
{
  show_numeric_host ( NULL );
  show_numeric_port ( NULL );
}

static void
set_proto ( char *arg )
{
  if ( !arg )
    fatal_config ( "Argument '-p' requere udp or tcp argument" );
  else if ( !strcasecmp ( arg, "tcp" ) )
    co.proto &= TCP;
  else if ( !strcasecmp ( arg, "udp" ) )
    co.proto &= UDP;
  else
    fatal_config ( "Invalid protocol in argument '-p'" );
}

static void
view_si ( UNUSED char *arg )
{
  co.view_si = true;
}

static void
verbose ( UNUSED char *arg )
{
  co.verbose = true;
}

static void
version ( UNUSED char *arg )
{
  show_version ();
  exit ( EXIT_SUCCESS );
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
  static const struct cmd cmd[] = { { "-B", "--bytes", view_bytes, NO_ARG },
                                    { "-c", "", view_conections, NO_ARG },
                                    { "", "--color", color_scheme, REQ_ARG },
                                    { "-f", "--file", log_file, OPT_ARG },
                                    { "-h", "--help", show_help, NO_ARG },
                                    { "-i", "--interface", iface, REQ_ARG },
                                    { "-n", "", show_numeric, NO_ARG },
                                    { "-nh", "", show_numeric_host, NO_ARG },
                                    { "-np", "", show_numeric_port, NO_ARG },
                                    { "-p", "--protocol", set_proto, REQ_ARG },
                                    { "", "--si", view_si, NO_ARG },
                                    { "-v", "--verbose", verbose, NO_ARG },
                                    { "-V", "--version", version, NO_ARG } };

  while ( --argc )
    {
      int check = 0;
      argv++;

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
    }

  return &co;
}
