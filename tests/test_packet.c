
#include <stdio.h>
#include <unistd.h> // sleep

#include "unity.h"
#include "../src/packet.c"

static void
test_dont_fragment ( void )
{
  struct iphdr ip = { .frag_off = htons ( IP_DF ) };
  int ret = is_first_frag ( &ip, NULL );

  TEST_ASSERT_EQUAL_INT ( 0, ret );

  ret = is_frag ( &ip );

  TEST_ASSERT_EQUAL_INT ( -1, ret );
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

  ret = is_first_frag ( &pkt_1.l3, &pkt_1.l4 );  // should be first frag
  TEST_ASSERT_EQUAL_INT ( 1, ret );

  ret = is_frag ( &pkt_1.l3 );
  TEST_ASSERT_EQUAL_INT ( -1, ret );  // shold be -1 because offset is 0

  pkt_1.l3.frag_off |= 1;
  ret = is_frag ( &pkt_1.l3 );  // now shold be different -1 because is one fragment
  TEST_ASSERT_EQUAL_INT ( 0, ret ); // first position of array pkt_ip_frag

  TEST_ASSERT_EQUAL_INT ( pkt_1.l4.source, pkt_ip_frag[ret].source_port );
  TEST_ASSERT_EQUAL_INT ( pkt_1.l4.dest, pkt_ip_frag[ret].dest_port );
  TEST_ASSERT_EQUAL_INT ( pkt_1.l3.id, pkt_ip_frag[ret].pkt_id );

  struct pkt pkt_2 = { .l3 = { .frag_off = htons ( IP_MF ),
                               .id = 2,
                               .saddr = 0x05050505,
                               .daddr = 0x06060606 },
                       .l4 = { .source = 1030, .dest = 443 } };

  ret = is_first_frag ( &pkt_2.l3, &pkt_2.l4 );  // should be first frag
  TEST_ASSERT_EQUAL_INT ( 1, ret );

  ret = is_frag ( &pkt_2.l3 );
  TEST_ASSERT_EQUAL_INT ( -1, ret );  // shold be -1 because offset is 0

  pkt_2.l3.frag_off |= 1;
  ret = is_frag ( &pkt_2.l3 );  // now shold be different -1 because is one fragment
  TEST_ASSERT_EQUAL_INT ( 1, ret ); // second position of array pkt_ip_frag

  TEST_ASSERT_EQUAL_INT ( pkt_2.l4.source, pkt_ip_frag[ret].source_port );
  TEST_ASSERT_EQUAL_INT ( pkt_2.l4.dest, pkt_ip_frag[ret].dest_port );
  TEST_ASSERT_EQUAL_INT ( pkt_2.l3.id, pkt_ip_frag[ret].pkt_id );
}

static void
test_clear_frag( void )
{
  struct pkt pkt = { .l3 = { .frag_off = htons ( IP_MF ),
                               .id = 3,
                               .saddr = 0x01010101,
                               .daddr = 0x02020202 },
                       .l4 = { .source = 1025, .dest = 80 } };

// store packet
TEST_ASSERT_EQUAL_INT ( 1, is_first_frag(&pkt.l3, &pkt.l4) );

pkt.l3.frag_off |= 1;
int ret = is_frag ( &pkt.l3 );
TEST_ASSERT_NOT_EQUAL_INT ( -1, ret );

pkt_ip_frag[ret].ttl -= LIFETIME_FRAG;

// packet end of life, not should be in array pkt_ip_frag
clear_frag();
TEST_ASSERT_EQUAL_INT ( -1, is_frag ( &pkt.l3 ) );

}

void
test_packet ( void )
{
  test_dont_fragment ();
  test_more_fragment ();
  test_clear_frag();
}
