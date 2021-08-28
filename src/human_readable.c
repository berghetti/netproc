
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
#include "config.h"
#include "sufix.h"

#define BASE_IEC 1024  // default
#define BASE_SI 1000

static unsigned int base;
static const char *const *sufix_rate;
static const char *const *sufix_total;

void
define_sufix ( const struct config_op *co )
{
  if ( co->view_si && co->view_bytes )
    {
      base = BASE_SI;
      sufix_rate = sufix_schemes[SI_BYTE];
      sufix_total = sufix_schemes[SI_BYTE_TOT];
    }
  else if ( co->view_si )
    {
      base = BASE_SI;
      sufix_rate = sufix_schemes[SI_BIT];
      sufix_total = sufix_schemes[SI_BIT_TOT];
    }
  else if ( co->view_bytes )
    {
      base = BASE_IEC;
      sufix_rate = sufix_schemes[IEC_BYTE];
      sufix_total = sufix_schemes[IEC_BYTE_TOT];
    }
  else
    {  // default
      base = BASE_IEC;
      sufix_rate = sufix_schemes[IEC_BIT];
      sufix_total = sufix_schemes[IEC_BIT_TOT];
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
  // double val = bytes;
  for ( size_t i = 1; i < TOT_ELEMENTS_SUFIX; i++ )
    {
      if ( ( bytes / base ) < base || i == ( TOT_ELEMENTS_SUFIX - 1 ) )
        {
          double val = (double) bytes / base;

          sn = snprintf ( buffer, len_buff, "%.2f %s", val, sufix[i] );
          return ( sn > 0 && ( size_t ) sn < len_buff );
        }

      bytes /= base;
    }

  return false; /* unreached */
}
