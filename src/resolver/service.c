
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

int
port2serv ( unsigned short int port,
            char *proto,
            char *buf,
            const size_t buf_len )
{
  struct servent *sve;

  char *protocol = ( proto ) ? proto : "tcp";

  // unsigned short int port_nb = htons(port);
  sve = getservbyport ( port, protocol );

  if ( sve != NULL )
    {
      fprintf(stderr, "%s\n", "translate port ok");
      strncpy ( buf, sve->s_name, buf_len );
      return 1;
    }
  else
    snprintf ( buf, buf_len, "%u", port );

  return 0;
}
