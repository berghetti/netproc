
/*
 *  Copyright (C) 2021 Mayco S. Berghetti
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

/* based implementation python
 https://github.com/python/cpython/blob/main/Python/hashtable.c */

#include <stdlib.h>
#include "slist.h"
#include "hashtable.h"

#define HASHTABLE_MIN_SIZE 16
#define HASHTABLE_HIGH 0.75
#define HASHTABLE_LOW 0.10
#define HASHTABLE_REHASH_FACTOR 2.0 / ( HASHTABLE_LOW + HASHTABLE_HIGH )

typedef struct hashtable
{
  size_t nentries;  // Total number of entries in the table
  size_t nbuckets;
  slist_t *buckets;

  func_hash fhash;        // callback hash function
  func_compare fcompare;  // callback compare keys
  func_clear fclear;      // callback clear data from user
} hashtable_t;

typedef struct hashtable_entry
{
  // used by hashtable_t.buckets to link entries
  slist_item_t _slist_item;  // "class parent"

  hash_t key_hash;
  void *key;
  void *value;
} hashtable_entry_t;

#define TABLE_HEAD( ht, index )                                           \
  ( ( hashtable_entry_t * ) ( ( slist_t * ) &( ht )->buckets[( index )] ) \
            ->head )

#define ENTRY_NEXT( entry ) \
  ( ( hashtable_entry_t * ) ( ( slist_item_t * ) ( entry ) )->next )

/* makes sure the real size of the buckets array is a power of 2
https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */
static inline size_t
next_power2 ( size_t s )
{
  s--;
  s |= s >> 1;
  s |= s >> 2;
  s |= s >> 4;
  s |= s >> 8;
  s |= s >> 16;

// 64 bits version
#if __SIZEOF_SIZE_T__ == 8
  s |= s >> 32;
#endif

  s++;

  return s;
}

static inline size_t
get_index ( hash_t hash, size_t size )
{
  return ( hash & ( size - 1 ) );
}

static bool
hashtable_rehash ( hashtable_t *ht )
{
  size_t num_buckets =
          next_power2 ( ( size_t ) ( ht->nentries * HASHTABLE_REHASH_FACTOR ) );

  if ( num_buckets == ht->nbuckets )
    return true;

  slist_t *new_buckets = calloc ( num_buckets, sizeof ( ht->buckets[0] ) );
  if ( !new_buckets )
    return false;

  for ( size_t i = 0; i < ht->nbuckets; i++ )
    {
      hashtable_entry_t *entry = TABLE_HEAD ( ht, i );
      while ( entry )
        {
          hashtable_entry_t *next = ENTRY_NEXT ( entry );

          size_t index = get_index ( entry->key_hash, num_buckets );
          slist_preprend ( &new_buckets[index], ( slist_item_t * ) entry );

          entry = next;
        }
    }

  free ( ht->buckets );
  ht->nbuckets = num_buckets;
  ht->buckets = new_buckets;

  return true;
}

hashtable_t *
hashtable_min_new ( void )
{
  hashtable_t *ht = malloc ( sizeof *ht );

  if ( !ht )
    return NULL;

  ht->nentries = 0;
  ht->nbuckets = HASHTABLE_MIN_SIZE;

  ht->buckets = calloc ( ht->nbuckets, sizeof ( ht->buckets[0] ) );
  if ( !ht->buckets )
    {
      free ( ht );
      return NULL;
    }

  return ht;
}

hashtable_t *
hashtable_new ( func_hash fhash, func_compare fcompare, func_clear fclear )
{
  hashtable_t *ht = hashtable_min_new ();

  if ( ht )
    {
      ht->fhash = fhash;
      ht->fcompare = fcompare;
      ht->fclear = fclear;
    }

  return ht;
}

void *
hashtable_min_set ( hashtable_t *ht,
                    void *value,
                    const void *key,
                    const hash_t hash )
{
  hashtable_entry_t *entry = malloc ( sizeof *entry );

  if ( !entry )
    return NULL;

  entry->key_hash = hash;
  entry->key = ( void * ) key;
  entry->value = value;

  ht->nentries++;
  if ( ( float ) ht->nentries / ( float ) ht->nbuckets > HASHTABLE_HIGH )
    {
      if ( !hashtable_rehash ( ( hashtable_t * ) ht ) )
        {
          ht->nentries--;
          free ( entry );
          return NULL;
        }
    }

  size_t index = get_index ( entry->key_hash, ht->nbuckets );
  slist_preprend ( &ht->buckets[index], ( slist_item_t * ) entry );

  return value;
}

void *
hashtable_set ( hashtable_t *restrict ht, const void *key, void *value )
{
  return hashtable_min_set ( ht, value, key, ht->fhash ( key ) );
}

static hashtable_entry_t *
get_entry ( hashtable_t *restrict ht,
            const void *restrict key,
            hash_t hash,
            func_compare cmp )
{
  size_t index = get_index ( hash, ht->nbuckets );

  hashtable_entry_t *entry = TABLE_HEAD ( ht, index );
  while ( entry )
    {
      if ( entry->key_hash == hash && cmp ( entry->key, key ) )
        break;

      entry = ENTRY_NEXT ( entry );
    }

  return entry;
}

void *
hashtable_min_get ( hashtable_t *restrict ht,
                    const void *restrict key,
                    hash_t hash,
                    func_compare cmp )
{
  hashtable_entry_t *entry = get_entry ( ht, key, hash, cmp );
  if ( entry )
    return entry->value;

  return entry;
}

void *
hashtable_get ( hashtable_t *restrict ht, const void *restrict key )
{
  hashtable_entry_t *entry =
          get_entry ( ht, key, ht->fhash ( key ), ht->fcompare );

  if ( entry )
    return entry->value;

  return entry;
}

size_t
hashtable_get_nentries ( hashtable_t *ht )
{
  return ht->nentries;
}

size_t
hashtable_get_size ( hashtable_t *ht )
{
  return ht->nbuckets;
}

int
hashtable_foreach ( hashtable_t *restrict ht,
                    hashtable_foreach_func func,
                    void *user_data )
{
  for ( size_t i = 0; i < ht->nbuckets; i++ )
    {
      hashtable_entry_t *entry = TABLE_HEAD ( ht, i );
      while ( entry )
        {
          hashtable_entry_t *entry_next = ENTRY_NEXT ( entry );

          int ret = func ( ht, entry->value, user_data );
          if ( ret )
            return ret;

          entry = entry_next;
        }
    }

  return 0;
}

void
hashtable_foreach_remove ( hashtable_t *restrict ht,
                           hashtable_foreach_func to_remove,
                           void *user_data )
{
  for ( size_t i = 0; i < ht->nbuckets; i++ )
    {
      hashtable_entry_t *entry = TABLE_HEAD ( ht, i );
      hashtable_entry_t *prev = NULL;
      while ( entry )
        {
          hashtable_entry_t *next = ENTRY_NEXT ( entry );

          if ( to_remove ( ( hashtable_t * ) ht, entry->value, user_data ) )
            {
              slist_remove ( &ht->buckets[i],
                             ( slist_item_t * ) prev,
                             ( slist_item_t * ) entry );
              free ( entry );
              ht->nentries--;
            }
          else
            {
              prev = entry;
            }

          entry = next;
        }
    }
}

void *
hashtable_min_remove ( hashtable_t *restrict ht,
                       const void *key,
                       hash_t hash,
                       func_compare cmp )
{
  size_t index = get_index ( hash, ht->nbuckets );

  hashtable_entry_t *entry = TABLE_HEAD ( ht, index );
  hashtable_entry_t *prev = NULL;
  while ( entry )
    {
      if ( entry->key_hash == hash && cmp ( entry->key, key ) )
        break;

      prev = entry;
      entry = ENTRY_NEXT ( entry );
    }

  if ( entry == NULL )
    return NULL;

  slist_remove ( &ht->buckets[index],
                 ( slist_item_t * ) prev,
                 ( slist_item_t * ) entry );

  void *value = entry->value;
  free ( entry );

  ht->nentries--;

  if ( ( float ) ht->nentries / ( float ) ht->nbuckets < HASHTABLE_LOW )
    hashtable_rehash ( ht );

  return value;
}

void *
hashtable_remove ( hashtable_t *restrict ht, const void *key )
{
  return hashtable_min_remove ( ht, key, ht->fhash ( key ), ht->fcompare );
}

static inline void
destroy_entry ( func_clear fclear, hashtable_entry_t *entry )
{
  if ( fclear )
    fclear ( entry->value );

  free ( entry );
}

void
hashtable_min_detroy ( hashtable_t *ht, func_clear fclear )
{
  for ( size_t i = 0; i < ht->nbuckets; i++ )
    {
      if ( !ht->nentries )
        break;

      hashtable_entry_t *entry = TABLE_HEAD ( ht, i );
      while ( entry )
        {
          hashtable_entry_t *entry_next = ENTRY_NEXT ( entry );
          destroy_entry ( fclear, entry );
          ht->nentries--;
          entry = entry_next;
        }
    }

  free ( ht->buckets );
  free ( ht );
}

void
hashtable_destroy ( hashtable_t *ht )
{
  hashtable_min_detroy ( ht, ht->fclear );
}
