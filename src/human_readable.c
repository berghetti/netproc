
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

#include <stdio.h>  // snprintf()
#include <stdbool.h>
#include <stdint.h>

#include "human_readable.h"

#define BASE_IEC 1024  // default
#define BASE_SI 1000

enum sufix_types
{
  IEC_BYTE = 0,
  IEC_BIT,
  SI_BYTE,
  SI_BIT,
  IEC_BYTE_TOT,
  IEC_BIT_TOT,
  SI_BYTE_TOT,
  SI_BIT_TOT,
  TOT_SUFIX_SCHEME
};

#define TOT_ELEMENTS_SUFIX 6

static const char *const sufix_schemes[TOT_SUFIX_SCHEME][TOT_ELEMENTS_SUFIX] = {
  [IEC_BYTE] = { "B/s", "KiB/s", "MiB/s", "GiB/s", "TiB/s", "PiB/s" },
  [IEC_BIT] = { "b/s", "Kib/s", "Mib/s", "Gib/s", "Tib/s", "Pib/s" },
  [SI_BYTE] = { "B/s", "KB/s", "MB/s", "GB/s", "TB/s", "PB/s" },
  [SI_BIT] = { "b/s", "Kb/s", "Mb/s", "Gb/s", "Tb/s", "Pb/s" },
  [IEC_BYTE_TOT] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB" },
  [IEC_BIT_TOT] = { "b", "Kib", "Mib", "Gib", "Tib", "Pib" },
  [SI_BYTE_TOT] = { "B", "KB", "MB", "GB", "TB", "PB" },
  [SI_BIT_TOT] = { "b", "Kb", "Mb", "Gb", "Tb", "Pb" }
};

static unsigned int base;
static const char *const *sufix_rate;
static const char *const *sufix_total;

void
define_sufix ( const bool view_si, const bool view_bytes )
{
  if ( view_si )
    {
      base = BASE_SI;

      if ( view_bytes )
        {
          sufix_rate = sufix_schemes[SI_BYTE];
          sufix_total = sufix_schemes[SI_BYTE_TOT];
        }
      else  // default
        {
          sufix_rate = sufix_schemes[SI_BIT];
          sufix_total = sufix_schemes[SI_BIT_TOT];
        }
    }
  else  // default
    {
      base = BASE_IEC;

      if ( view_bytes )
        {
          sufix_rate = sufix_schemes[IEC_BYTE];
          sufix_total = sufix_schemes[IEC_BYTE_TOT];
        }
      else  // default
        {
          sufix_rate = sufix_schemes[IEC_BIT];
          sufix_total = sufix_schemes[IEC_BIT_TOT];
        }
    }
}

// based in source code of program wget
// https://github.com/mirror/wget/blob/master/src/utils.c#L1675

bool
human_readable ( char *buffer, size_t len_buff, uint64_t bytes, int mode )
{
  const char *const *sufix = ( mode == RATE ) ? sufix_rate : sufix_total;

  ssize_t sn;

  if ( !bytes )
    {
      sn = snprintf ( buffer, len_buff, "%c", '0' );
      return ( sn == 1 );
    }

  // quantidade de bytes ou bits menor que 1024 ou 1000
  if ( bytes < base )
    {
      sn = snprintf ( buffer, len_buff, "%ld %s", bytes, sufix[0] );
      return ( sn > 0 && ( size_t ) sn < len_buff );
    }

  /* a cada loop os bytes/bits recebidos são divididos por sua base (1000 ou
   1024) quando o valor for menor que sua base, ou ja estejamos no ultimo
   elemento do array de sufixos, temos a melhor aproximação com o sufixo
   apropriado. */
  for ( size_t i = 1; i < TOT_ELEMENTS_SUFIX; i++ )
    {
      if ( ( bytes / base ) < base || i == ( TOT_ELEMENTS_SUFIX - 1 ) )
        {
          double val = ( double ) bytes / base;

          sn = snprintf ( buffer, len_buff, "%.2f %s", val, sufix[i] );
          return ( sn > 0 && ( size_t ) sn < len_buff );
        }

      bytes /= base;
    }

  return false; /* unreached */
}
