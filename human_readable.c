
#include <stdio.h>      // snprintf
#include <stdint.h>
#include <stdbool.h>

#include "sufix.h"      //define chosen_base, sufix and LEN_ARR_SUFIX

// based in source code of program wget
// https://github.com/mirror/wget/blob/master/src/utils.c


// caso valor final obtido seja maior ou igual que accuracy
// não será impresso casas decimais
// 1024 é melhor valor pois só não ira exibir casas decimais quando o fluxo
// for maior ou igual 1024 Tib ou TiB... :p
// caso o valor fosse 1000, poderiamos ter resultados como
// 1023 Kb/s, quando seria mais apropriado 1023.00 Kb/s
#define ACCURACY 1024

// numero de casas decimais que serão inseridas caso
// o valor seja menor que ACCURACY (maioria dos casos, creio que sempre)
#define DECIMAL_PLACES 2


bool human_readable (char *buffer, size_t len_buff, uint64_t bytes)
{
  ssize_t sn;
  int decimals;


  const size_t base = chosen_base;
  const size_t len_sufix = LEN_ARR_SUFIX;

  // quantidade de bytes ou bits menor que 1024 ou 1000
  if (bytes < base)
    {
      sn = snprintf (buffer, len_buff, "%d %s", (int) bytes, sufix[0]);
      return (sn > 0 && (size_t) sn < len_buff);
    }

// comentário original wget
/* Loop over powers, dividing N with 1024 in each iteration.  This
   works unchanged for all sizes of wgint, while still avoiding
   non-portable `long double' arithmetic.  */
// a cada loop os bytes/bits recebidos são divididos por sua base (1000 ou 1024)
// quando o valor for menor que sua base, ou ja estejamos no ultimo elemento
// do array de sufixos, temos a melhor aproximação com o sufixo apropriado.
  size_t i;
  for (i = 1; i < len_sufix; i++)
    {

      /* At each iteration N is greater than the *subsequent* power.
         That way N/1024.0 produces a decimal number in the units of
         *this* power.  */
      if ((bytes / base) < base || i == (len_sufix - 1))
        {
          long double val = (double) bytes / base;
          decimals = ( (uint32_t) val < ACCURACY) ? DECIMAL_PLACES : 0;
          // printf("val - %Lf\n", val);
          /* Print values smaller than the accuracy level (acc) with (decimal)
           * decimal digits, and others without any decimals.  */
          sn = snprintf (buffer, len_buff, "%.*Lf %s", decimals, val, sufix[i]);
          return (sn > 0 && (size_t) sn < len_buff);
        }
      bytes /= base;
    }
  return false;                  /* unreached */
}
