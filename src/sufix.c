
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

#include <stdbool.h>

#include "sufix.h"

// #define TOT_ELEMENTS_SUFIX 6

enum sufix_types
{
  SUFIX_IEC_B = 0,
  SUFIX_IEC_b,
  SUFIX_SI_B,
  SUFIX_SI_b,
  SUFIX_IEC_B_TOT,
  SUFIX_IEC_b_TOT,
  SUFIX_SI_B_TOT,
  SUFIX_SI_b_TOT,
  TOT_SUFIX_SCHEME
};

static const char *const sufix_schemes[TOT_SUFIX_SCHEME][TOT_ELEMENTS_SUFIX] = {
        [SUFIX_IEC_B] = {"B/s", "KiB/s", "MiB/s", "GiB/s", "TiB/s", "PiB/s"},
        [SUFIX_IEC_b] = {"b/s", "Kib/s", "Mib/s", "Gib/s", "Tib/s", "Pib/s"},
        [SUFIX_SI_B] = {"B/s", "KB/s", "MB/s", "GB/s", "TB/s", "PB/s"},
        [SUFIX_SI_b] = {"b/s", "Kb/s", "Mb/s", "Gb/s", "Tb/s", "Pb/s"},
        [SUFIX_IEC_B_TOT] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB/s"},
        [SUFIX_IEC_b_TOT] = {"b", "Kib", "Mib", "Gib", "Tib", "Pib/s"},
        [SUFIX_SI_B_TOT] = {"B", "KB", "MB", "GB", "TB", "PB"},
        [SUFIX_SI_b_TOT] = {"b", "Kb", "Mb", "Gb", "Tb", "Pb"}};

// defined in main.c
extern bool view_si;
extern bool view_bytes;

int chosen_base;
const char *const *sufix_rate;
const char *const *sufix_total;

void
define_sufix ( void )
{
  if ( view_si && view_bytes )
    {
      chosen_base = BASE_SI;
      sufix_rate = sufix_schemes[SUFIX_SI_B];
      sufix_total = sufix_schemes[SUFIX_SI_B_TOT];
    }
  else if ( view_si )
    {
      chosen_base = BASE_SI;
      sufix_rate = sufix_schemes[SUFIX_SI_b];
      sufix_total = sufix_schemes[SUFIX_SI_b_TOT];
    }
  else if ( view_bytes )
    {
      chosen_base = BASE_IEC;
      sufix_rate = sufix_schemes[SUFIX_IEC_B];
      sufix_total = sufix_schemes[SUFIX_IEC_B_TOT];
    }
  else
    {  // default
      chosen_base = BASE_IEC;
      sufix_rate = sufix_schemes[SUFIX_IEC_b];
      sufix_total = sufix_schemes[SUFIX_IEC_b_TOT];
    }
}
