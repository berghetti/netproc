
/*
 *  Copyright (C) 2020 Mayco S. Berghetti
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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

typedef struct conection
{
  struct net_stat net_stat;  // armazena statisticas de rede
  uint32_t if_index;
  uint32_t inode;
  uint32_t local_address;
  uint32_t remote_address;
  uint16_t local_port;
  uint16_t remote_port;
  uint8_t protocol;
} conection_t;

int
get_conections_system ( conection_t **buffer, const int proto );

#endif  // CONECTION_H
