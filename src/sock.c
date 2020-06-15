
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

#include <stdlib.h>           // malloc
#include <arpa/inet.h>        // htons
#include <errno.h>            // variable errno
#include <linux/if_ether.h>   // defined ETH_P_ALL
#include <linux/if_packet.h>  // struct sockaddr_ll
#include <net/if.h>           // if_nametoindex
#include <string.h>           // strerror
#include <sys/socket.h>       // socket
#include <sys/mman.h>         // mmap
#include <sys/types.h>        // socket
#include <fcntl.h>            // fcntl
#include <unistd.h>           // close

#include <assert.h>           // testes

#include "sock.h"
#include "m_error.h"



// socket timeout default in microseconds
// 1 second          <--> 1E+6 microseconds
// 1E+5 microseconds <--> 1/10 second
// #define TIMEOUT 1E+5

// struct block_desc {
// 	uint32_t version;
// 	uint32_t offset_to_priv;
// 	struct tpacket_hdr_v1 h1;
// };
//
// struct ring {
// 	struct iovec *rd;
// 	uint8_t *map;
// 	struct tpacket_req3 req;
// };

// defined in main.c
extern char *iface;

static void
create_buff( struct ring *ring, int sock );

static void
map_buff( struct ring *ring, int sock );

static void
socket_setnonblocking( int sock );

static void
bind_interface ( const char *iface, int sock );

// static void
// set_timeout ( void );

// void rotation_buffer(void)
// {
//   frame_idx = (frame_idx + 1) % req.tp_frame_nr;
//   int buffer_idx = frame_idx / frames_per_block;
//   char* buffer_ptr = rx_ring + buffer_idx * req.tp_block_size;
//   int frame_idx_diff = frame_idx % frames_per_block;
//   frame_ptr = buffer_ptr + frame_idx_diff * req.tp_frame_size;
// }

int
create_socket ( struct ring *ring )
{
  int sock;

  if ( ( sock = socket ( AF_PACKET, SOCK_RAW, htons ( ETH_P_ALL ) ) ) == -1 )
    fatal_error ( "Error create socket: %s", strerror ( errno ) );

  socket_setnonblocking( sock );

  create_buff( ring, sock );

  map_buff( ring, sock );
  // set_timeout ();
  bind_interface ( iface, sock );

  return sock;
}

void
close_socket ( int sock )
{
  if ( sock > 0 )
    close ( sock );
}

static void
socket_setnonblocking( int sock )
{
  int flag;

  if ( ( flag = fcntl ( sock, F_GETFL ) ) == -1 )
    fatal_error ( "Cannot get socket flags: \"%s\"", strerror ( errno ) );

  if ( fcntl ( sock, F_SETFL, flag | O_NONBLOCK ) == -1 )
    fatal_error ( "Cannot set socket to non-blocking mode: \"%s\"", strerror ( errno ) );
}


static void
create_buff( struct ring *ring, int sock )
{
  ring->req.tp_frame_size = TPACKET_ALIGN(TPACKET3_HDRLEN + ETH_HLEN) +
                                                        TPACKET_ALIGN(2000);
  // tamanho inicial de uma pagina de memoria
  ring->req.tp_block_size = sysconf(_SC_PAGESIZE);
  // dobra o tamanho do bloco até que caiba um frame_size
  while (ring->req.tp_block_size < ring->req.tp_frame_size * 20) {
    ring->req.tp_block_size <<= 1;
   }
  ring->req.tp_block_nr = 64;
  size_t frames_per_block = ring->req.tp_block_size / ring->req.tp_frame_size;
  ring->req.tp_frame_nr = ring->req.tp_block_nr * frames_per_block;
  ring->req.tp_retire_blk_tov = 60;
  ring->req.tp_feature_req_word = 0;

  // set version TPACKET
  int version = TPACKET_V3;
  if (setsockopt(sock, SOL_PACKET, PACKET_VERSION, &version, sizeof(version)) == -1)
    fatal_error("setsockopt version: %s", strerror ( errno ));

  // set conf buffer tpacket
  if (setsockopt(sock, SOL_PACKET, PACKET_RX_RING, &ring->req, sizeof(ring->req)) == -1 )
    fatal_error("setsockopt: %s", strerror ( errno ));
}

static void
map_buff( struct ring *ring, int sock )
{
  size_t rx_ring_size = ring->req.tp_block_nr * ring->req.tp_block_size;
  ring->map = mmap(0, rx_ring_size, PROT_READ|PROT_WRITE, MAP_SHARED, sock, 0);
  if (ring->map == MAP_FAILED)
    fatal_error("mmap: %s", strerror ( errno ));


  ring->rd = malloc(ring->req.tp_block_nr * sizeof(*ring->rd));
	assert(ring->rd);
	for (size_t i = 0; i < ring->req.tp_block_nr; ++i) {
		ring->rd[i].iov_base = ring->map + (i * ring->req.tp_block_size);
		ring->rd[i].iov_len = ring->req.tp_block_size;
	}

  // frame_ptr = ring->map;
}

// static void
// set_timeout ( void )
// {
//   struct timeval read_timeout;
//   read_timeout.tv_sec = 0;
//   read_timeout.tv_usec = TIMEOUT;
//
//   // set timeout for read in socket
//   if ( setsockopt ( sock,
//                     SOL_SOCKET,
//                     SO_RCVTIMEO,
//                     &read_timeout,
//                     sizeof ( read_timeout ) ) == -1 )
//     fatal_error ( "Error set timeout socket: %s", strerror ( errno ) );
// }

static void
bind_interface ( const char *iface, int sock )
{
  struct sockaddr_ll my_sock = {0};
  my_sock.sll_family = AF_PACKET;
  my_sock.sll_protocol = htons ( ETH_P_ALL );

  // 0 match all interfaces
  if ( !iface )
    my_sock.sll_ifindex = 0;
  else
    {
      if ( !( my_sock.sll_ifindex = if_nametoindex ( iface ) ) )
        fatal_error ( "Interface: %s", strerror ( errno ) );
    }

  if ( bind ( sock, ( struct sockaddr * ) &my_sock, sizeof ( my_sock ) ) == -1 )
    fatal_error ( "Error bind interface %s", strerror ( errno ) );
}
