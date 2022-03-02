
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

#ifndef CONECTION_H
#define CONECTION_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "rate.h"  // struct net_stat

// stores the information exported by the kernel in /proc/net/tcp | udp
typedef struct conection
{
  struct net_stat net_stat;  // assign in statistics.c
  /* kernel linux usage this type to inode
  https://elixir.bootlin.com/linux/v5.10.19/source/net/core/sock.c#L2161 */
  unsigned long inode;
  bool active;
  int if_index;  // assign in statistics.c
  uint32_t local_address;
  uint32_t remote_address;
  uint16_t local_port;
  uint16_t remote_port;
  uint8_t protocol;
  uint8_t state;
} connection_t;

bool
connection_init ( void );

bool
connection_update ( const int proto );

connection_t *
connection_get ( const unsigned long inode );

void
connection_free ( void );

#endif  // CONECTION_H
