
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

int
port2serv ( unsigned short int port,
            const char *proto,
            char *buf,
            const size_t buf_len )
{
  struct servent *sve;

  memset ( buf, 0, buf_len );

  sve = getservbyport ( htons ( port ), proto );

  if ( sve != NULL )
    {
      strncpy ( buf, sve->s_name, buf_len );
      return 1;
    }
  else
    snprintf ( buf, buf_len, "%u", port );

  return 0;
}
