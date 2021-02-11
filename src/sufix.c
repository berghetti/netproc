
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
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

#include <stdbool.h>

#include "sufix.h"

enum sufix_types
{
  SUFIX_IEC_BYTE = 0,
  SUFIX_IEC_BIT,
  SUFIX_SI_BYTE,
  SUFIX_SI_BIT,
  SUFIX_IEC_BYTE_TOT,
  SUFIX_IEC_BIT_TOT,
  SUFIX_SI_BYTE_TOT,
  SUFIX_SI_BIT_TOT,
  TOT_SUFIX_SCHEME
};

static const char *const sufix_schemes[TOT_SUFIX_SCHEME][TOT_ELEMENTS_SUFIX] = {
        [SUFIX_IEC_BYTE] = {"B/s", "KiB/s", "MiB/s", "GiB/s", "TiB/s", "PiB/s"},
        [SUFIX_IEC_BIT] = {"b/s", "Kib/s", "Mib/s", "Gib/s", "Tib/s", "Pib/s"},
        [SUFIX_SI_BYTE] = {"B/s", "KB/s", "MB/s", "GB/s", "TB/s", "PB/s"},
        [SUFIX_SI_BIT] = {"b/s", "Kb/s", "Mb/s", "Gb/s", "Tb/s", "Pb/s"},
        [SUFIX_IEC_BYTE_TOT] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"},
        [SUFIX_IEC_BIT_TOT] = {"b", "Kib", "Mib", "Gib", "Tib", "Pib"},
        [SUFIX_SI_BYTE_TOT] = {"B", "KB", "MB", "GB", "TB", "PB"},
        [SUFIX_SI_BIT_TOT] = {"b", "Kb", "Mb", "Gb", "Tb", "Pb"}};

int chosen_base;
const char *const *sufix_rate;
const char *const *sufix_total;

void
define_sufix ( const struct config_op *co )
{
  if ( co->view_si && co->view_bytes )
    {
      chosen_base = BASE_SI;
      sufix_rate = sufix_schemes[SUFIX_SI_BYTE];
      sufix_total = sufix_schemes[SUFIX_SI_BYTE_TOT];
    }
  else if ( co->view_si )
    {
      chosen_base = BASE_SI;
      sufix_rate = sufix_schemes[SUFIX_SI_BIT];
      sufix_total = sufix_schemes[SUFIX_SI_BIT_TOT];
    }
  else if ( co->view_bytes )
    {
      chosen_base = BASE_IEC;
      sufix_rate = sufix_schemes[SUFIX_IEC_BYTE];
      sufix_total = sufix_schemes[SUFIX_IEC_BYTE_TOT];
    }
  else
    {  // default
      chosen_base = BASE_IEC;
      sufix_rate = sufix_schemes[SUFIX_IEC_BIT];
      sufix_total = sufix_schemes[SUFIX_IEC_BIT_TOT];
    }
}
