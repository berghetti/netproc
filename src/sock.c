
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
#include <sys/types.h>        // socket
#include <fcntl.h>            // fcntl
#include <unistd.h>           // close

#include "sock.h"
#include "m_error.h"

// // quantidade de blocos
// #define N_BLOCKS 64
//
// /* essa conf influencia o uso do tempo da CPU */
// #define FRAMES_PER_BLOCK 20
//
// // tamanho do frame (pacote), descosiderando o overhead do cabeçalho tpacket
// // seria uma boa ajustar conforme o maior MTU (tirando a loopback)
// #define LEN_FRAME 1600
//
// // timeout in miliseconds
// #define TIMEOUT_FRAME 65

// defined in main.c
extern char *iface;

// static void
// create_buff ( int sock, struct ring *ring );

// static void
// map_buff ( int sock, struct ring *ring );

static void
socket_setnonblocking ( int sock );

static void
bind_interface ( int sock, const char *iface );

int
create_socket ( void )
{
  int sock;

  if ( ( sock = socket ( AF_PACKET, SOCK_RAW, htons ( ETH_P_ALL ) ) ) == -1 )
    fatal_error ( "Error create socket: %s", strerror ( errno ) );

  socket_setnonblocking ( sock );

  // create_buff ( sock, ring );
  //
  // map_buff ( sock, ring );

  bind_interface ( sock, iface );

  return sock;
}

void
close_socket ( int sock )
{
  if ( sock > 0 )
    close ( sock );
}

static void
socket_setnonblocking ( int sock )
{
  int flag;

  if ( ( flag = fcntl ( sock, F_GETFL ) ) == -1 )
    fatal_error ( "Cannot get socket flags: \"%s\"", strerror ( errno ) );

  if ( fcntl ( sock, F_SETFL, flag | O_NONBLOCK ) == -1 )
    fatal_error ( "Cannot set socket to non-blocking mode: \"%s\"",
                  strerror ( errno ) );
}

// static void
// create_buff ( int sock, struct ring *ring )
// {
//   ring->req.tp_frame_size = TPACKET_ALIGN ( TPACKET3_HDRLEN + ETH_HLEN ) +
//                             TPACKET_ALIGN ( LEN_FRAME );
//   // tamanho inicial de uma pagina de memoria
//   ring->req.tp_block_size = sysconf ( _SC_PAGESIZE );
//   // dobra o tamanho do bloco até que caiba um frame_size
//   while ( ring->req.tp_block_size < ring->req.tp_frame_size * FRAMES_PER_BLOCK )
//     {
//       ring->req.tp_block_size <<= 1;
//     }
//
//   ring->req.tp_block_nr = N_BLOCKS;
//   size_t frames_per_block = ring->req.tp_block_size / ring->req.tp_frame_size;
//   ring->req.tp_frame_nr = ring->req.tp_block_nr * frames_per_block;
//   ring->req.tp_retire_blk_tov = TIMEOUT_FRAME;
//   ring->req.tp_feature_req_word = 0;
//
//   // set version TPACKET
//   int version = TPACKET_V3;
//   if ( setsockopt ( sock,
//                     SOL_PACKET,
//                     PACKET_VERSION,
//                     &version,
//                     sizeof ( version ) ) == -1 )
//     fatal_error ( "setsockopt version: %s", strerror ( errno ) );
//
//   // set conf buffer tpacket
//   if ( setsockopt ( sock,
//                     SOL_PACKET,
//                     PACKET_RX_RING,
//                     &ring->req,
//                     sizeof ( ring->req ) ) == -1 )
//     fatal_error ( "setsockopt: %s", strerror ( errno ) );
// }
//
// static void
// map_buff ( int sock, struct ring *ring )
// {
//   size_t rx_ring_size = ring->req.tp_block_nr * ring->req.tp_block_size;
//   ring->map =
//           mmap ( 0, rx_ring_size, PROT_READ | PROT_WRITE, MAP_SHARED, sock, 0 );
//   if ( ring->map == MAP_FAILED )
//     fatal_error ( "mmap: %s", strerror ( errno ) );
//
//   ring->rd = calloc ( ring->req.tp_block_nr, sizeof ( *ring->rd ) );
//   if ( !ring->rd )
//     fatal_error ( "calloc: %s", strerror ( errno ) );
//
//   for ( size_t i = 0; i < ring->req.tp_block_nr; ++i )
//     {
//       ring->rd[i].iov_base = ring->map + ( i * ring->req.tp_block_size );
//       ring->rd[i].iov_len = ring->req.tp_block_size;
//     }
// }

static void
bind_interface ( int sock, const char *iface )
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
