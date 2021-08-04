
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RING_H
#define RING_H

#include <sys/uio.h>          // struct iovec
#include <linux/if_packet.h>  // strct tpacket_req3

struct ring
{
  struct tpacket_req3 req;
  struct iovec *rd;
  uint8_t *map;
};

struct ring *
ring_init ( int sock );

void
ring_free ( struct ring *ring );

#endif  // RING_H
