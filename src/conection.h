
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
  struct net_stat net_stat;  // this assign in src/statistics.c
  unsigned long int inode;
  uint32_t if_index;  // this assign in src/statistics.c
  uint32_t local_address;
  uint32_t remote_address;
  uint16_t local_port;
  uint16_t remote_port;
  uint8_t protocol;
  uint8_t state;
} conection_t;

int
get_conections ( conection_t **buffer, const int proto );

#endif  // CONECTION_H
