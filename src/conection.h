
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

// FIXME: ... value is ideal???
// tamanho do buffer para identificar as conexões do sistema
#define MAX_CONECTIONS 8000

// caminho do arquivo onde o kernel
// fornece as conexoes TCP e UDP
#define PATH_TCP "/proc/net/tcp"
#define PATH_UDP "/proc/net/udp"

// struct net_stat;

typedef struct conection
{
  struct net_stat net_stat;  // armazena statisticas de rede
  uint32_t inode;
  uint32_t local_address;
  uint32_t remote_address;
  uint16_t local_port;
  uint16_t remote_port;
} conection_t;

// get conections of system in "/proc/net/tcp or udp"
int
get_info_conections ( conection_t *conection, const size_t lenght );

// preenche o buffer com indices apenas das conexões que estão ativas
// @param size *buffer - armazena os indices correspondentes do array
// conection_t * que contem as conexões ativas
// @param conection *con - o array de conexões para checar
// @param size_t tot_con - tamanho do array para checar
// size_t
// get_con_active_in_process ( size_t *buffer,
//                             const conection_t *con,
//                             const size_t tot_con );

#endif  // CONECTION_H
