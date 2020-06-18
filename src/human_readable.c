
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
#include <stdint.h>
#include <stdio.h>  // snprintf()

#include "human_readable.h"
#include "integer.h"  // is_integer()
#include "sufix.h"    // define chosen_base, sufix, sufix_tot,
                      // LEN_ARR_SUFIX

// based in source code of program wget
// https://github.com/mirror/wget/blob/master/src/utils.c

// caso valor final obtido seja maior ou igual que accuracy
// não será impresso casas decimais
// 1024 é o melhor valor pois só não ira exibir casas decimais quando o fluxo
// for maior ou igual 1024 ? (? == ultimo array do array sufix),
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

  // retorno da função snprintf
  ssize_t sn;

  // se 0, não mostra sufixo
  if ( !bytes )
    {
      sn = snprintf ( buffer, len_buff, "%d", ( int ) bytes );
      return ( sn > 0 && ( size_t ) sn < len_buff );
    }

  // quantidade de bytes ou bits menor que 1024 ou 1000
  if ( bytes < ( uint64_t ) chosen_base )
    {
      sn = snprintf ( buffer, len_buff, "%d %s", ( int ) bytes, sufix[0] );
      return ( sn > 0 && ( size_t ) sn < len_buff );
    }

  // a cada loop os bytes/bits recebidos são divididos por sua base (1000 ou
  // 1024) quando o valor for menor que sua base, ou ja estejamos no ultimo
  // elemento do array de sufixos, temos a melhor aproximação com o sufixo
  // apropriado.

  // pega elemento inverso da base escolhida
  const double base = INVERSE_BASE ( chosen_base );
  int decimals;
  double val;

  // not necessary this variable, only val is necessary,
  // but make the code more readable
  double bytest = bytes;

  for ( size_t i = 1; i < TOT_ELEMENTS_SUFIX; i++ )
    {
      /* At each iteration N is greater than the *subsequent* power.
         That way N/1024.0 produces a decimal number in the units of
         *this* power.  */
      if ( ( val = bytest * base ) < chosen_base ||
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
