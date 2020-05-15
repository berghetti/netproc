#include <stdio.h>
#include "usage.h"

void
show_version(void)
{
  puts(PROG_NAME " - " PROG_VERSION);
}

void
usage ( void )
{
  show_version();

  puts ( "Usage: " PROG_NAME " [options]\n"
         "\n"
         "Options:\n"
         "-u            rastreia trafego udp, padrão é tcp\n"
         "-i <iface>    seleciona a interface, padrão é todas\n"
         "-B            visualização em bytes, padrão em bits\n"
         "-si           visualização com formato SI, com potências de 1000\n"
         "              padrão é IEC, com potências de 1024\n"
         "-h            exibe essa mensagem\n"
         "-v            exibe a versão"
         );
}
