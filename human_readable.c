
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>  // snprintf()

#include "integer.h"  // is_integer()
#include "sufix.h"    //define chosen_base, sufix and LEN_ARR_SUFIX

// based in source code of program wget
// https://github.com/mirror/wget/blob/master/src/utils.c

// caso valor final obtido seja maior ou igual que accuracy
// não será impresso casas decimais
// 1023 é o melhor valor pois só não ira exibir casas decimais quando o fluxo
// for maior ou igual 1024 Tib ou TiB... :p
// caso o valor fosse 1000, poderiamos ter resultados como
// 1023 Kb/s, quando seria mais apropriado 1023.50 Kb/s
#define ACCURACY 1024

// numero de casas decimais que serão inseridas caso
// o valor seja menor que ACCURACY (maioria dos casos, creio que sempre)
#define DECIMAL_PLACES 2

bool
human_readable ( char *buffer, const size_t len_buff, uint64_t bytes )
{
  // retorno da função snprintf
  ssize_t sn;

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

  // up: alterado divisão por multiplicação para melhor otimização
  // pelo compilador

  const double base = INVERSE_BASE ( chosen_base );
  int decimals;
  double val;
  for ( size_t i = 1; i < LEN_ARR_SUFIX; i++ )
    {
      /* At each iteration N is greater than the *subsequent* power.
         That way N/1024.0 produces a decimal number in the units of
         *this* power.  */
      if ( ( val = bytes * base ) < chosen_base || i == ( LEN_ARR_SUFIX - 1 ) )
        {
          // coloca casas decimais se o valor for menor que ACCURACY
          // e se não for inteiro um valor inteiro

          //FIXME: alguns arredondamentos do printf ainda não batem com a função
          // is_integer, e acaba exibindo casas decimais para "valores inteiros"
          // considerados por printf
          decimals =
              ( ( uint32_t ) val < ACCURACY )
                  ? ! is_integer ( val, DECIMAL_PLACES, 1 ) ? DECIMAL_PLACES : 0
                  : 0;

          /* Print values smaller than the accuracy level (acc) with (decimal)
           * decimal digits, and others without any decimals.  */
          sn =
              snprintf ( buffer, len_buff, "%.*f %s", decimals, val, sufix[i] );
          return ( sn > 0 && ( size_t ) sn < len_buff );
        }

      bytes *= base;
    }
  return false; /* unreached */
}
