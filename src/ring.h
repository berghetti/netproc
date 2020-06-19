#ifndef RING_H
#define RING_H

#include <sys/uio.h>  // struct iovec

struct ring
{
  struct tpacket_req3 req;
  struct iovec *rd;
  uint8_t *map;
};

void
create_ring ( int sock, struct ring *ring );

void
free_ring ( struct ring *ring );

#endif  // RING_H
