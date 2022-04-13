
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

// based implementation python
// https://github.com/python/cpython/blob/main/Python/hashtable.c

#include <stdlib.h>
#include "hashtable.h"

#define HASHTABLE_MIN_SIZE 16
#define HASHTABLE_HIGH 0.75
#define HASHTABLE_LOW 0.10
#define HASHTABLE_REHASH_FACTOR 2.0 / ( HASHTABLE_LOW + HASHTABLE_HIGH )

#define TABLE_HEAD( ht, index )                                           \
  ( ( hashtable_entry_t * ) ( ( slist_t * ) &( ht )->buckets[( index )] ) \
            ->head )

#define PP_TABLE_HEAD( ht, index )                                          \
  ( ( hashtable_entry_t ** ) &( ( slist_t * ) &( ht )->buckets[( index )] ) \
            ->head )

#define ENTRY_NEXT( entry ) \
  ( ( hashtable_entry_t * ) ( ( slist_item_t * ) ( entry ) )->next )

#define PP_ENTRY_NEXT( entry ) \
  ( ( hashtable_entry_t ** ) &( ( slist_item_t * ) ( entry ) )->next )

// makes sure the real size of the buckets array is a power of 2
// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static inline size_t
next_power2 ( size_t s )
{
  // if ( s < HASHTABLE_MIN_SIZE )
  //   return HASHTABLE_MIN_SIZE;
  //
  // size_t ns = HASHTABLE_MIN_SIZE;
  //
  // while ( ns < s )
  //   ns <<= 1;

  s--;
  s |= s >> 1;
  s |= s >> 2;
  s |= s >> 4;
  s |= s >> 8;
  s |= s >> 16;

#if __SIZEOF_SIZE_T__ == 8
  s |= s >> 32;
#endif

  s++;

  return s;
}

static inline void
hashtable_preprend ( slist_t *list, slist_item_t *item )
{
  item->next = list->head;
  list->head = item;
}

static inline size_t
get_index ( hash_t hash, size_t size )
{
  return ( hash & ( size - 1 ) );
}

static int
hashtable_rehash ( hashtable_t *ht )
{
  size_t num_buckets =
          next_power2 ( ( size_t ) ( ht->nentries * HASHTABLE_REHASH_FACTOR ) );

  if ( num_buckets == ht->nbuckets )
    return 1;

  slist_t *new_buckets = calloc ( num_buckets, sizeof ( ht->buckets[0] ) );
  if ( !new_buckets )
    return 0;

  for ( size_t i = 0; i < ht->nbuckets; i++ )
    {
      hashtable_entry_t *entry = TABLE_HEAD ( ht, i );
      while ( entry )
        {
          hashtable_entry_t *next = ENTRY_NEXT ( entry );

          size_t index = get_index ( entry->key_hash, num_buckets );
          hashtable_preprend ( &new_buckets[index], ( slist_item_t * ) entry );

          entry = next;
        }
    }

  free ( ht->buckets );
  ht->nbuckets = num_buckets;
  ht->buckets = new_buckets;

  return 1;
}

static hashtable_entry_t *
hashtable_get_entry ( hashtable_t *restrict ht, const void *restrict key )
{
  hash_t hash = ht->fhash ( key );
  size_t index = get_index ( hash, ht->nbuckets );

  hashtable_entry_t *entry = TABLE_HEAD ( ht, index );
  while ( entry )
    {
      if ( entry->key_hash == hash && ht->fcompare ( entry->key, key ) )
        break;

      entry = ENTRY_NEXT ( entry );
    }

  return entry;
}

hashtable_t *
hashtable_new ( func_hash fhash, func_compare fcompare, func_clear fclear )
{
  hashtable_t *ht = malloc ( sizeof *ht );

  if ( !ht )
    return NULL;

  ht->nbuckets = HASHTABLE_MIN_SIZE;

  ht->buckets = calloc ( ht->nbuckets, sizeof ( ht->buckets[0] ) );
  if ( !ht->buckets )
    {
      free ( ht );
      return NULL;
    }

  ht->nentries = 0;
  ht->fhash = fhash;
  ht->fcompare = fcompare;
  ht->fclear = fclear;

  return ht;
}

void *
hashtable_set ( hashtable_t *restrict ht, const void *key, void *value )
{
  hashtable_entry_t *entry = malloc ( sizeof *entry );
  if ( !entry )
    return NULL;

  entry->key_hash = ht->fhash ( key );
  entry->key = ( void * ) key;
  entry->value = value;

  ht->nentries++;
  if ( ( float ) ht->nentries / ( float ) ht->nbuckets > HASHTABLE_HIGH )
    {
      if ( !hashtable_rehash ( ht ) )
        {
          ht->nentries--;
          free ( entry );
          return NULL;
        }
    }

  size_t index = get_index ( entry->key_hash, ht->nbuckets );
  hashtable_preprend ( &ht->buckets[index], ( slist_item_t * ) entry );

  return value;
}

void *
hashtable_get ( hashtable_t *restrict ht, const void *restrict key )
{
  hashtable_entry_t *entry = hashtable_get_entry ( ht, key );
  if ( entry )
    return entry->value;

  return entry;
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

// https://github.com/mkirchner/linked-list-good-taste/
static void *
remove_entry ( hashtable_t *restrict ht, const void *key )
{
  hash_t hash = ht->fhash ( key );
  size_t index = get_index ( hash, ht->nbuckets );

  hashtable_entry_t **entry = PP_TABLE_HEAD ( ht, index );
  while ( *entry )
    {
      if ( ( *entry )->key_hash == hash &&
           ht->fcompare ( ( *entry )->key, key ) )
        break;

      entry = PP_ENTRY_NEXT ( *entry );
    }

  if ( *entry == NULL )
    return NULL;

  hashtable_entry_t *del = *entry;
  void *value = del->value;

  *entry = ENTRY_NEXT ( del );
  free ( del );

  ht->nentries--;

  return value;
}

void *
hashtable_remove ( hashtable_t *restrict ht, const void *key )
{

  void *res = remove_entry ( ht, key );

  if ( res )
    {
      if ( ( float ) ht->nentries / ( float ) ht->nbuckets < HASHTABLE_LOW )
        hashtable_rehash ( ht );
    }

  return res;
}

void *
hashtable_simple_remove ( hashtable_t *restrict ht, const void *key )
{
  return remove_entry ( ht, key );
}

static inline void
hashtable_destroy_entry ( hashtable_t *ht, hashtable_entry_t *entry )
{
  if ( ht->fclear )
    ht->fclear ( entry->value );

  free ( entry );
}

void
hashtable_destroy ( hashtable_t *ht )
{
  for ( size_t i = 0; i < ht->nbuckets; i++ )
    {
      if ( !ht->nentries )
        break;

      hashtable_entry_t *entry = TABLE_HEAD ( ht, i );
      while ( entry )
        {
          hashtable_entry_t *entry_next = ENTRY_NEXT ( entry );
          hashtable_destroy_entry ( ht, entry );
          ht->nentries--;
          entry = entry_next;
        }
    }

  free ( ht->buckets );
  free ( ht );
}

///////////////////////

hashtable_min *
hashtable_min_new ( void )
{
  hashtable_min *ht = malloc ( sizeof *ht );

  if ( !ht )
    return NULL;

  ht->nbuckets = HASHTABLE_MIN_SIZE;
  ht->buckets = calloc ( ht->nbuckets, sizeof ( ht->buckets[0] ) );

  if ( !ht->buckets )
    {
      free ( ht );
      return NULL;
    }

  ht->nentries = 0;

  return ht;
}

void *
hashtable_min_set ( hashtable_min *ht,
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
  hashtable_preprend ( &ht->buckets[index], ( slist_item_t * ) entry );

  return value;
}

static hashtable_entry_t *
hashtable_min_get_entry ( hashtable_min *restrict ht,
                          const void *key,
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
hashtable_min_get ( hashtable_min *restrict ht,
                    const void *key,
                    hash_t hash,
                    func_compare cmp )
{
  hashtable_entry_t *entry = hashtable_min_get_entry ( ht, key, hash, cmp );

  if ( entry )
    return entry->value;

  return NULL;
}

int
hashtable_min_foreach ( hashtable_min *restrict ht,
                        hashtable_foreach_func func,
                        void *user_data )
{
  for ( size_t i = 0; i < ht->nbuckets; i++ )
    {
      hashtable_entry_t *entry = TABLE_HEAD ( ht, i );
      while ( entry )
        {
          hashtable_entry_t *entry_next = ENTRY_NEXT ( entry );

          int ret = func ( ( hashtable_t * ) ht, entry->value, user_data );
          if ( ret )
            return ret;

          entry = entry_next;
        }
    }

  return 0;
}

static void *
hashtable_min_remove_entry ( hashtable_t *restrict ht,
                             const void *key,
                             hash_t hash,
                             func_compare cmp )
{
  size_t index = get_index ( hash, ht->nbuckets );

  hashtable_entry_t **entry = PP_TABLE_HEAD ( ht, index );
  while ( *entry )
    {
      if ( ( *entry )->key_hash == hash && cmp ( ( *entry )->key, key ) )
        break;

      entry = PP_ENTRY_NEXT ( *entry );
    }

  if ( *entry == NULL )
    return NULL;

  hashtable_entry_t *del = *entry;
  void *value = del->value;

  *entry = ENTRY_NEXT ( del );
  free ( del );

  ht->nentries--;

  return value;
}

void *
hashtable_min_remove ( hashtable_t *ht,
                       const void *key,
                       hash_t hash,
                       func_compare cmp )
{

  void *res = hashtable_min_remove_entry ( ht, key, hash, cmp );

  if ( res )
    {
      if ( ( float ) ht->nentries / ( float ) ht->nbuckets < HASHTABLE_LOW )
        hashtable_rehash ( ht );
    }

  return res;
}

void *
hashtable_min_simple_remove ( hashtable_t *ht,
                              const void *key,
                              hash_t hash,
                              func_compare cmp )
{
  return hashtable_min_remove_entry ( ht, key, hash, cmp );
}

static inline void
hashtable_min_destroy_entry ( func_clear fclear, hashtable_entry_t *entry )
{
  if ( fclear )
    fclear ( entry->value );

  free ( entry );
}

void
hashtable_min_detroy ( hashtable_min *ht, func_clear fclear )
{
  for ( size_t i = 0; i < ht->nbuckets; i++ )
    {
      if ( !ht->nentries )
        break;

      hashtable_entry_t *entry = TABLE_HEAD ( ht, i );
      while ( entry )
        {
          hashtable_entry_t *entry_next = ENTRY_NEXT ( entry );
          hashtable_min_destroy_entry ( fclear, entry );
          ht->nentries--;
          entry = entry_next;
        }
    }

  free ( ht->buckets );
  free ( ht );
}
