
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

// based in source code of program wget
// https://github.com/mirror/wget/blob/master/src/utils.c

#include <stdio.h>    // snprintf()
#include <stdbool.h>
#include <stdint.h>

#include "human_readable.h"
#include "config.h"
#include "integer.h"  // is_integer()
#include "sufix.h"

static int base;
static const char *const *sufix_rate;
static const char *const *sufix_total;

void
define_sufix ( const struct config_op *co )
{
  if ( co->view_si && co->view_bytes )
    {
      base = BASE_SI;
      sufix_rate = sufix_schemes[SUFIX_SI_BYTE];
      sufix_total = sufix_schemes[SUFIX_SI_BYTE_TOT];
    }
  else if ( co->view_si )
    {
      base = BASE_SI;
      sufix_rate = sufix_schemes[SUFIX_SI_BIT];
      sufix_total = sufix_schemes[SUFIX_SI_BIT_TOT];
    }
  else if ( co->view_bytes )
    {
      base = BASE_IEC;
      sufix_rate = sufix_schemes[SUFIX_IEC_BYTE];
      sufix_total = sufix_schemes[SUFIX_IEC_BYTE_TOT];
    }
  else
    {  // default
      base = BASE_IEC;
      sufix_rate = sufix_schemes[SUFIX_IEC_BIT];
      sufix_total = sufix_schemes[SUFIX_IEC_BIT_TOT];
    }
}

// caso valor final obtido seja maior ou igual que accuracy
// não será impresso casas decimais
// 1024 é o melhor valor pois só não ira exibir casas decimais quando o fluxo
// for maior ou igual 1024 ? (? == ultimo elemento do array sufix),
// atualmento Tib/s, caso o valor fosse 1000, poderiamos ter resultados como
// 1023 Kb/s, quando seria mais apropriado 1023.50 Kb/s
#define ACCURACY 1024

// numero de casas decimais que serão inseridas caso
// o valor seja menor que ACCURACY (maioria dos casos, creio que sempre)
#define DECIMAL_PLACES 2

bool
human_readable ( char *buffer,
                 const size_t len_buff,
                 const uint64_t bytes,
                 int mode )
{
  const char *const *sufix;
  if ( mode == RATE )
    sufix = sufix_rate;
  else
    sufix = sufix_total;

  ssize_t sn;

  if ( !bytes )
    {
      sn = snprintf ( buffer, len_buff, "%c", '0' );
      return ( sn > 0 && ( size_t ) sn < len_buff );
    }

  // quantidade de bytes ou bits menor que 1024 ou 1000
  if ( bytes < ( uint64_t ) base )
    {
      sn = snprintf ( buffer, len_buff, "%d %s", ( int ) bytes, sufix[0] );
      return ( sn > 0 && ( size_t ) sn < len_buff );
    }

  // a cada loop os bytes/bits recebidos são divididos por sua base (1000 ou
  // 1024) quando o valor for menor que sua base, ou ja estejamos no ultimo
  // elemento do array de sufixos, temos a melhor aproximação com o sufixo
  // apropriado.

  // pega elemento inverso da base escolhida
  const double ibase = INVERSE_BASE ( base );
  int decimals;
  double val;

  double bytest = bytes;
  for ( size_t i = 1; i < TOT_ELEMENTS_SUFIX; i++ )
    {
      /* At each iteration N is greater than the *subsequent* power.
         That way N/1024.0 produces a decimal number in the units of
         *this* power.  */
      if ( ( val = bytest * ibase ) < base ||
           i == ( TOT_ELEMENTS_SUFIX - 1 ) )
        {
          // coloca casas decimais se o valor for menor que ACCURACY
          // e se não for um valor inteiro
          decimals = ( ( uint32_t ) val < ACCURACY )
                             ? !is_integer ( val, DECIMAL_PLACES, 1 )
                                       ? DECIMAL_PLACES
                                       : 0
                             : 0;

          /* Print values smaller than the accuracy level (acc) with (decimal)
           * decimal digits, and others without any decimals.  */
          sn = snprintf (
                  buffer, len_buff, "%.*f %s", decimals, val, sufix[i] );
          return ( sn > 0 && ( size_t ) sn < len_buff );
        }

      bytest = val;
    }

  return false; /* unreached */
}
