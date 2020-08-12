
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

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

struct config_op
{
  char *iface;             // bind interface
  int *color_scheme;       // scheme colors
  bool udp;                // TCP or UDP
  bool view_si;            // SI or IEC prefix
  bool view_bytes;         // view in bytes or bits
  bool view_conections;    // show conections each process
  bool translate_host;     // translate ip to name using DNS
  bool translate_service;  // translate port to service

  uint8_t tic_tac;  // sinc program, internal control
};

struct config_op *
parse_options ( int argc, const char **argv );

#endif  // CONFIG_H
