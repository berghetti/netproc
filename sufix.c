
#include <stdbool.h>

#include "sufix.h"

// sufixos padrão IEC, com potências de 2, 1024, 2048..., com bytes
static const char *const sufix_iec_B[] = {"B/s",
                                          "KiB/s",
                                          "MiB/s",
                                          "GiB/s",
                                          "TiB/s"};

// sufixos padrão IEC com bits
// default
static const char *const sufix_iec_b[] = {"b/s",
                                          "Kib/s",
                                          "Mib/s",
                                          "Gib/s",
                                          "Tib/s"};

// sufixos padrão SI, com potências de 10. 1000, 2000..., com bytes
static const char *const sufix_si_B[] = {"B/s", "KB/s", "MB/s", "GB/s", "TB/s"};

// sufixos padrão SI com bits
static const char *const sufix_si_b[] = {"b/s", "Kb/s", "Mb/s", "Gb/s", "Tb/s"};

// defined in main.c
extern bool view_si;
extern bool view_bytes;

int chosen_base;
const char *const *sufix;

void
define_sufix ( void )
{
  if ( view_si && view_bytes )
    {
      chosen_base = BASE_SI;
      sufix = sufix_si_B;
    }
  else if ( view_si )
    {
      chosen_base = BASE_SI;
      sufix = sufix_si_b;
    }
  else if ( view_bytes )
    {
      chosen_base = BASE_IEC;
      sufix = sufix_iec_B;
    }
  else
    {  // default
      chosen_base = BASE_IEC;
      sufix = sufix_iec_b;
    }
}
