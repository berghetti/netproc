
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

#ifndef NETWORK_H
#define NETWORK_H

#include <linux/if_packet.h>  // struct sockaddr_ll
#include <stdint.h>           // types uint*_t
#include <sys/types.h>        // type ssize_t
#include "sock.h"

#ifndef IPPROTO_UDP
#define IPPROTO_UDP 17
#endif

// values of packet.direction
#define PKT_DOWN 1
#define PKT_UPL 2

// used for function parse_packet e statistics
struct packet
{
  size_t lenght;
  uint32_t if_index;
  uint32_t local_address;
  uint32_t remote_address;
  uint16_t local_port;
  uint16_t remote_port;
  uint8_t direction;
  uint8_t protocol;
};

// preenche a struct packet com os dados do pacote recebido
int
parse_packet ( struct packet *restrict pkt, struct tpacket3_hdr *restrict ppd );

#endif  // NETWORK_H
