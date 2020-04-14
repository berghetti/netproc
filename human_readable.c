
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h> // PRIu64

#include "process.h"

// inspired in source code of program wget

/* The number of elements in an array.  For example:
   static char a[] = "foo";     -- countof(a) == 4 (note terminating \0)
   int a[5] = {1, 2};           -- countof(a) == 5
   char *a[] = {                -- countof(a) == 3
     "foo", "bar", "baz"
   }; */
#define n_elements(array) (sizeof (array) / sizeof ((array)[0]))

// 1024 sistema IEC, base 2
// 1000 sistema SI, base 10


// @ n - bytes á computar
// @ acc - não exibe casas decimais caso resultado seja maior ou igual este valor
// @ decimals - numero de casa decimais que sera colocado
char * human_readable (char *buffer, size_t len_buff, uint64_t n)
{
  const int acc = 1024;   // melhor valor, não alterar
  const int decimals = 2;
  // printf("bytes recebidos - %"PRIu64"\n", n);
  /* These suffixes are compatible with those of GNU `ls -lh'. */
  const char *const powers[] = {" KB/s", " MB/s", " GB/s", " TB/s" };
  // const char *const powers[] = {" Kb/s", " Mb/s", " Gb/s", " Tb/s" };

  // 'M',                      /* megabyte, 2^20 bytes */
  // 'G',                      /* gigabyte, 2^30 bytes */
  // 'T',                      /* terabyte, 2^40 bytes */
  // 'P',                      /* petabyte, 2^50 bytes */
  // 'E',                      /* exabyte,  2^60 bytes */

  static char buf[10];


  /* If the quantity is smaller than 1K, just print it. */
  if (n < 1024)
    {
      snprintf (buffer, len_buff, "%d%s", (int) n, " Bps");
      return buf;
    }

  /* Loop over powers, dividing N with 1024 in each iteration.  This
     works unchanged for all sizes of wgint, while still avoiding
     non-portable `long double' arithmetic.  */
  size_t i;
  for (i = 0; i < n_elements (powers); i++)
    {
      /* At each iteration N is greater than the *subsequent* power.
         That way N/1024.0 produces a decimal number in the units of
         *this* power.  */
      if ((n / 1024) < 1024 || i == n_elements (powers) - 1)
        {
          long double val = n / 1024.0L;
          // printf("val - %Lf\n", val);
          /* Print values smaller than the accuracy level (acc) with (decimal)
           * decimal digits, and others without any decimals.  */
          snprintf (buffer, len_buff, "%.*Lf%s",
                    val < acc ? decimals : 0, val, powers[i]);
          return buf;
        }
      n /= 1024;
    }
  return NULL;                  /* unreached */
}


// int main(int argc, char **argv)
// {
//   printf("argv[1] - %llu\n", atoll(argv[1]));
//
//   char *down_size = human_readable (atoll(argv[1]), 1000, 2);
//
//
//   printf("%.0f\n", 3.1559238);
//
//   printf("bytes - %s\n", down_size);
// }
