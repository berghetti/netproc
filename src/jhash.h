#ifndef JHASH_H
#define JHASH_H

/* jhash.h: Jenkins hash support.
 *
 * Copyright (C) 2006. Bob Jenkins (bob_jenkins@burtleburtle.net)
 *
 * https://burtleburtle.net/bob/hash/
 *
 * These are the credits from Bob's sources:
 *
 * lookup3.c, by Bob Jenkins, May 2006, Public Domain.
 *
 * These are functions for producing 32-bit hashes for hash table lookup.
 * hashword(), hashlittle(), hashlittle2(), hashbig(), mix(), and final()
 * are externally useful functions.  Routines to test the hash are included
 * if SELF_TEST is defined.  You can use this free for any purpose.  It's in
 * the public domain.  It has no warranty.
 *
 * --
 * Edited to useful to netproc
 * Copyright (C) 2022 Mayco S. Berghetti
 */

#include <stdint.h>
#include "macro_util.h"

#define rot( x, k ) ( ( ( x ) << ( k ) ) | ( ( x ) >> ( 32 - ( k ) ) ) )

// clang-format off
/* JHASH_MIX -- mix 3 32-bit values reversibly. */
#define JHASH_MIX(a, b, c)			    \
{						                        \
	a -= c;  a ^= rot(c, 4);  c += b;	\
	b -= a;  b ^= rot(a, 6);  a += c;	\
	c -= b;  c ^= rot(b, 8);  b += a;	\
	a -= c;  a ^= rot(c, 16); c += b;	\
	b -= a;  b ^= rot(a, 19); a += c;	\
	c -= b;  c ^= rot(b, 4);  b += a;	\
}

/* JHASH_FINAL - final mixing of 3 32-bit values (a,b,c) into c */
#define JHASH_FINAL(a, b, c)	\
{						                  \
	c ^= b; c -= rot(b, 14);		\
	a ^= c; a -= rot(c, 11);		\
	b ^= a; b -= rot(a, 25);		\
	c ^= b; c -= rot(b, 16);		\
	a ^= c; a -= rot(c, 4);		  \
	b ^= a; b -= rot(a, 14);		\
	c ^= b; c -= rot(b, 24);		\
}

/* An arbitrary initial parameter */
#define JHASH_INITVAL 0xDEADBEEF

/* jhash8 - hash an arbitrary key
 * @k: sequence of bytes as key
 * @length: the length of the key
 * @initval: the previous hash, or an arbitray value
 *
 * The generic version, hashes an arbitrary sequence of bytes.
 * No alignment or length assumptions are made about the input key.
 *
 * Returns the hash value of the key. The result depends on endianness.
 */
static inline uint32_t
jhash8 ( const void *key, uint32_t length, uint32_t initval )
{
  uint32_t a, b, c;
  const uint8_t *k = ( const uint8_t * ) key;

  /* Set up the internal state */
  a = b = c = JHASH_INITVAL + length + initval;

  /* All but the last block: affect some 32 bits of (a,b,c) */
  while ( length > 12 )
    {
      a += *k;
      b += *( k + 4 );
      c += *( k + 8 );
      JHASH_MIX ( a, b, c );
      length -= 12;
      k += 12;
    }

  /* Last block: affect all 32 bits of (c) */
  switch ( length )
    {
      case 12: c += ( uint32_t ) k[11] << 24; FALLTHROUGH;
      case 11: c += ( uint32_t ) k[10] << 16; FALLTHROUGH;
      case 10: c += ( uint32_t ) k[9] << 8; FALLTHROUGH;
      case 9: c += k[8]; FALLTHROUGH;
      case 8: b += ( uint32_t ) k[7] << 24; FALLTHROUGH;
      case 7: b += ( uint32_t ) k[6] << 16; FALLTHROUGH;
      case 6: b += ( uint32_t ) k[5] << 8; FALLTHROUGH;
      case 5: b += k[4]; FALLTHROUGH;
      case 4: a += ( uint32_t ) k[3] << 24; FALLTHROUGH;
      case 3: a += ( uint32_t ) k[2] << 16; FALLTHROUGH;
      case 2: a += ( uint32_t ) k[1] << 8; FALLTHROUGH;
      case 1:
        a += k[0];
        JHASH_FINAL ( a, b, c );
    }

  return c;
}

/* jhash32 - hash an array of uint32_t's
 * @k: the key which must be an array of uint32_t's
 * @length: the number of uint32_t's in the key
 * @initval: the previous hash, or an arbitray value
 *
 * Returns the hash value of the key.
 */
static inline uint32_t
jhash32 ( const uint32_t *k, uint32_t length, uint32_t initval )
{
  uint32_t a, b, c;

  /* Set up the internal state */
  a = b = c = JHASH_INITVAL + ( length << 2 ) + initval;

  /* Handle most of the key */
  while ( length > 3 )
    {
      a += k[0];
      b += k[1];
      c += k[2];
      JHASH_MIX ( a, b, c );
      length -= 3;
      k += 3;
    }

  /* Handle the last 3 uint32_t's */
  switch ( length )
    {
      case 3: c += k[2]; FALLTHROUGH;
      case 2: b += k[1]; FALLTHROUGH;
      case 1:
        a += k[0];
        JHASH_FINAL ( a, b, c );
    }

  return c;
}

// clang-format on
#endif /* JHASH_H */
