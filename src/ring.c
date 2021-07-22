
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

#include <stdlib.h>  // calloc
#include <inttypes.h>
#include <sys/socket.h>       // setsockopt
#include <linux/if_packet.h>  // *PACKET*
#include <linux/if_ether.h>   // ETH_HLEN
#include <sys/mman.h>         // mmap
#include <unistd.h>           // sysconf

#include "ring.h"
#include "m_error.h"

// reference
// https://www.kernel.org/doc/html/latest/networking/packet_mmap.html

#ifndef TPACKET3_HDRLEN
#error "TPACKET_V3 is necessary, check kernel linux version"
#endif

// each block will have at least 256KiB of size, because 128 * 2048 = 256KiB
// this considering a page size of 4096.
// this conf influences the use of CPU time and mmemory usage
// keep it as power-of-two

// quantidade de blocos
#define N_BLOCKS 4

#define FRAMES_PER_BLOCK 128  // 128 32
// size of frame (packet), considering overhead of struct tpacket (80 bytes)
// size small cause more usage CPU
#define LEN_FRAME 2048
// #define LEN_FRAME 262144

// timeout in miliseconds
// zero means that the kernel will calculate the timeout
// https://github.com/torvalds/linux/blob/master/net/packet/af_packet.c#L596
#define TIMEOUT_FRAME 0

static bool
create_ring_buff ( struct ring *ring )
{
  long page_size;
  size_t frames_per_block;

  ring->req.tp_frame_size = LEN_FRAME;
  // TPACKET_ALIGN ( TPACKET3_HDRLEN ) + TPACKET_ALIGN ( LEN_FRAME );

  // tamanho inicial de uma pagina de memoria
  errno = 0;
  if ( -1 == ( page_size = sysconf ( _SC_PAGESIZE ) ) )
    {
      ERROR_DEBUG ( "%s", ( errno ? strerror ( errno ) : "Error sysconf" ) );
      return false;
    }

  // The block has to be page size aligned
  // dobra o tamanho do bloco até que caiba a quantidade de frames
  ring->req.tp_block_size = page_size;
  while ( ring->req.tp_block_size < ring->req.tp_frame_size * FRAMES_PER_BLOCK )
    {
      ring->req.tp_block_size <<= 1;
    }

  ring->req.tp_block_nr = N_BLOCKS;
  frames_per_block = ring->req.tp_block_size / ring->req.tp_frame_size;
  ring->req.tp_frame_nr = ring->req.tp_block_nr * frames_per_block;
  ring->req.tp_retire_blk_tov = TIMEOUT_FRAME;
  ring->req.tp_feature_req_word = 0;

  return true;
}

static bool
config_ring ( int sock, struct ring *ring, int version )
{
  // set version TPACKET
  if ( setsockopt ( sock,
                    SOL_PACKET,
                    PACKET_VERSION,
                    &version,
                    sizeof ( version ) ) == -1 )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return false;
    }

  // set conf buffer tpacket
  if ( setsockopt ( sock,
                    SOL_PACKET,
                    PACKET_RX_RING,
                    &ring->req,
                    sizeof ( ring->req ) ) == -1 )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return false;
    }

  return true;
}

static bool
map_buff ( int sock, struct ring *ring )
{
  size_t rx_ring_size = ring->req.tp_block_nr * ring->req.tp_block_size;
  ring->map =
          mmap ( 0, rx_ring_size, PROT_READ | PROT_WRITE, MAP_SHARED, sock, 0 );

  if ( ring->map == MAP_FAILED )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return false;
    }

  ring->rd = calloc ( sizeof ( *ring->rd ), ring->req.tp_block_nr );
  if ( !ring->rd )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return false;
    }

  for ( size_t i = 0; i < ring->req.tp_block_nr; ++i )
    {
      ring->rd[i].iov_base = ring->map + ( i * ring->req.tp_block_size );
      ring->rd[i].iov_len = ring->req.tp_block_size;
    }

  return true;
}

bool
ring_init ( int sock, struct ring *ring )
{
  if ( !create_ring_buff ( ring ) )
    return false;

  if ( !config_ring ( sock, ring, TPACKET_V3 ) )
    return false;

  if ( !map_buff ( sock, ring ) )
    return false;

  return true;
}

void
ring_free ( struct ring *ring )
{
  munmap ( ring->map, ring->req.tp_block_size * ring->req.tp_block_nr );

  if ( ring->rd )
    free ( ring->rd );
}
