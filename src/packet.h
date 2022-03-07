
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

#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>           // types uint*_t
#include <sys/socket.h>       // setsockopt
#include <linux/if_packet.h>  // struct tpacket3_hdr

#include "sockaddr.h"

// used for function parse_packet e statistics
struct packet
{
  struct tuple tuple;  // source and dest ip/port
  uint32_t lenght;     // lenght of packet
  int if_index;        // interface index
  uint8_t direction;   // tx or rx
};

// packet.direction
#define PKT_DOWN 1
#define PKT_UPL 2

// preenche a struct packet com os dados do pacote recebido
int
parse_packet ( struct packet *pkt, struct tpacket3_hdr *ppd );

#endif  // NETWORK_H
