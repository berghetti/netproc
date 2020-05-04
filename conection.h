#ifndef CONECTION_H
#define CONECTION_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// checar isso...
#define MAX_CONECTIONS 1024

// caminho do arquivo onde o kernel
// fornece as conexoes TCP e UDP
#define PATH_TCP "/proc/net/tcp"
#define PATH_UDP "/proc/net/udp"

typedef struct
{
  uint32_t inode;
  uint32_t local_address;
  uint32_t remote_address;
  uint16_t local_port;
  uint16_t remote_port;
} conection_t;

int
get_info_conections ( conection_t *conection, const size_t lenght );

#endif  // CONECTION_H
