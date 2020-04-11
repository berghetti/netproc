#ifndef CONECTION_H
#define CONECTION_H

#include <stdlib.h>
#include <stdint.h>


typedef struct
{
  uint32_t inode;
  uint32_t local_address;
  uint32_t remote_address;
  uint16_t local_port;
  uint16_t remote_port;
}conection_t;

int get_info_conections(conection_t *conection,
                    const size_t lenght,
                    const char *conection_file);

#endif //CONECTION_H
