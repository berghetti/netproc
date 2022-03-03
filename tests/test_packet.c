
#include <stdio.h>
#include <unistd.h>  // sleep

#include "unity.h"
#include "../src/packet.c"

static void
test_dont_fragment ( void )
{
  struct iphdr ip = { .frag_off = htons ( IP_DF ) };

  TEST_ASSERT_EQUAL_INT ( -1, get_fragment ( &ip, NULL ) );
}

struct pkt
{
  struct iphdr l3;
  struct layer_4 l4;
};

static void
test_more_fragment ( void )
{
  int ret;

  struct pkt pkt_1 = { .l3 = { .frag_off = htons ( IP_MF ),
                               .id = 1,
                               .saddr = 0x01010101,
                               .daddr = 0x02020202 },
                       .l4 = { .source = 1025, .dest = 80 } };

  ret = get_fragment ( &pkt_1.l3, &pkt_1.l4 );  // should be first frag
  TEST_ASSERT_EQUAL_INT ( 0, ret );  // first position array pkt_ip_frag

  // test else if in get_fragment
  pkt_1.l3.frag_off |= 1;
  ret = get_fragment (
          &pkt_1.l3,
          &pkt_1.l4 );  // now shold be different -1 because is one fragment
  TEST_ASSERT_EQUAL_INT ( 0, ret );  // first position of array pkt_ip_frag

  TEST_ASSERT_EQUAL_INT ( pkt_1.l4.source, pkt_ip_frag[ret].source_port );
  TEST_ASSERT_EQUAL_INT ( pkt_1.l4.dest, pkt_ip_frag[ret].dest_port );
  TEST_ASSERT_EQUAL_INT ( pkt_1.l3.id, pkt_ip_frag[ret].id );

  struct pkt pkt_2 = {
    .l3 = { .frag_off = htons ( IP_MF ),
            .id = 1,  // same id pkt_1, but address differents
            .saddr = 0x05050505,
            .daddr = 0x06060606 },
    .l4 = { .source = 1030, .dest = 443 }
  };

  ret = get_fragment ( &pkt_2.l3, &pkt_2.l4 );
  TEST_ASSERT_EQUAL_INT ( 1, ret );  // second position of array pkt_ip_frag

  pkt_2.l3.frag_off |= 1;
  ret = get_fragment ( &pkt_2.l3, &pkt_1.l4 );
  TEST_ASSERT_EQUAL_INT ( 1, ret );

  TEST_ASSERT_EQUAL_INT ( pkt_2.l4.source, pkt_ip_frag[ret].source_port );
  TEST_ASSERT_EQUAL_INT ( pkt_2.l4.dest, pkt_ip_frag[ret].dest_port );
  TEST_ASSERT_EQUAL_INT ( pkt_2.l3.id, pkt_ip_frag[ret].id );
}

static void
test_clear_frag ( void )
{
  struct pkt pkt = { .l3 = { .frag_off = htons ( IP_MF ),
                             .id = 3,
                             .saddr = 0x01010101,
                             .daddr = 0x02020202 },
                     .l4 = { .source = 1025, .dest = 80 } };

  int ret = get_fragment ( &pkt.l3, &pkt.l4 );
  TEST_ASSERT_NOT_EQUAL_INT ( -1, ret );  // should be a fragment

  // make a fragment
  pkt.l3.frag_off |= 1;
  pkt_ip_frag[ret].ttl -= LIFETIME_FRAG;
  clear_frag ();  // should remove packet of array of fragments

  pkt.l3.frag_off = 0;
  TEST_ASSERT_EQUAL_INT ( -1, get_fragment ( &pkt.l3, &pkt.l4 ) );
}

static void
test_err_fragment ( void )
{
  struct pkt pkt = { .l3 = { .frag_off = htons ( IP_MF ),
                             .id = 3,
                             .saddr = 0x01010101,
                             .daddr = 0x02020202 },
                     .l4 = { .source = 1025, .dest = 80 } };

  pkt.l3.frag_off |= 1;  // make this a fragment whith offset diferente de 0

  // is a fragment, but as offset is not 0, not was stored in buffer
  TEST_ASSERT_EQUAL_INT ( ERR_FRAGMENT, get_fragment ( &pkt.l3, &pkt.l4 ) );
}

void
test_packet ( void )
{
  test_dont_fragment ();
  test_more_fragment ();
  test_clear_frag ();
  test_err_fragment ();
}
