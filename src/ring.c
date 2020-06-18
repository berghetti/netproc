
#include <stdlib.h>           // calloc
#include <inttypes.h>
#include <sys/socket.h>       // setsockopt
#include <linux/if_packet.h>  // *PACKET*
#include <linux/if_ether.h>   // ETH_HLEN
#include <sys/mman.h>         // mmap
#include <unistd.h>           // sysconf

#include "ring.h"
#include "m_error.h"

#ifndef TPACKET3_HDRLEN
#error "TPACKET_V3 is necessary, check kernel linux version"
#endif

// quantidade de blocos
#define N_BLOCKS 64

/* essa conf influencia o uso do tempo da CPU */
#define FRAMES_PER_BLOCK 20

// tamanho do frame (pacote), descosiderando o overhead do cabeçalho tpacket
// sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr)
// payload not important
#define LEN_FRAME ETH_HLEN + 20 + 20

// timeout in miliseconds
#define TIMEOUT_FRAME 65

static void
create_ring_buff ( struct ring *ring )
{
  ring->req.tp_frame_size = TPACKET_ALIGN ( TPACKET3_HDRLEN ) +
                            TPACKET_ALIGN ( LEN_FRAME );
  // tamanho inicial de uma pagina de memoria
  ring->req.tp_block_size = sysconf ( _SC_PAGESIZE );
  // dobra o tamanho do bloco até que caiba um frame_size
  while ( ring->req.tp_block_size < ring->req.tp_frame_size * FRAMES_PER_BLOCK )
    {
      ring->req.tp_block_size <<= 1;
    }

  ring->req.tp_block_nr = N_BLOCKS;
  size_t frames_per_block = ring->req.tp_block_size / ring->req.tp_frame_size;
  ring->req.tp_frame_nr = ring->req.tp_block_nr * frames_per_block;
  ring->req.tp_retire_blk_tov = TIMEOUT_FRAME;
  ring->req.tp_feature_req_word = 0;

}

static void
config_ring(int sock, struct ring *ring, int version)
{
  // set version TPACKET
  if ( setsockopt ( sock,
                    SOL_PACKET,
                    PACKET_VERSION,
                    &version,
                    sizeof ( version ) ) == -1 )
    fatal_error ( "setsockopt version: %s", strerror ( errno ) );

  // set conf buffer tpacket
  if ( setsockopt ( sock,
                    SOL_PACKET,
                    PACKET_RX_RING,
                    &ring->req,
                    sizeof ( ring->req ) ) == -1 )
    fatal_error ( "setsockopt: %s", strerror ( errno ) );
}

static void
map_buff ( int sock, struct ring *ring )
{
  size_t rx_ring_size = ring->req.tp_block_nr * ring->req.tp_block_size;
  ring->map =
          mmap ( 0, rx_ring_size, PROT_READ | PROT_WRITE, MAP_SHARED, sock, 0 );
  if ( ring->map == MAP_FAILED )
    fatal_error ( "mmap: %s", strerror ( errno ) );

  ring->rd = calloc ( ring->req.tp_block_nr, sizeof ( *ring->rd ) );
  if ( !ring->rd )
    fatal_error ( "calloc: %s", strerror ( errno ) );

  for ( size_t i = 0; i < ring->req.tp_block_nr; ++i )
    {
      ring->rd[i].iov_base = ring->map + ( i * ring->req.tp_block_size );
      ring->rd[i].iov_len = ring->req.tp_block_size;
    }
}

void
create_ring(int sock, struct ring *ring)
{
  create_ring_buff(ring);

  config_ring(sock, ring, TPACKET_V3);

  map_buff(sock, ring);
}

void
free_ring(struct ring *ring)
{
  munmap(ring->map, ring->req.tp_block_size * ring->req.tp_block_nr);
  free(ring->rd);
}
