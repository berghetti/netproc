
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

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#define PROG_NAME "netproc"
#define PROG_NAME_LOG PROG_NAME ".log"

#define PROG_VERSION "0.5.5"

// values field proto in struct config_op
#define TCP ( 1 << 0 )
#define UDP ( 1 << 1 )

struct config_op
{
  char *iface;             // bind interface
  char *path_log;          // path to log in file
  int *color_scheme;       // scheme colors
  int proto;               // tcp or udp
  bool log;                // log in file
  bool view_si;            // SI or IEC prefix
  bool view_bytes;         // view in bytes or bits
  bool view_conections;    // show conections each process
  bool translate_host;     // translate ip to name using DNS
  bool translate_service;  // translate port to service
  bool verbose;            // show process without traffic alse

  // internal
  uint8_t tic_tac;          // sinc program, internal control
  double running;           // time the program is running
  uint32_t max_digits_pid;  // used for spacing between columns in show.c
};

struct config_op *
parse_options ( int argc, char **argv );

#endif  // CONFIG_H
