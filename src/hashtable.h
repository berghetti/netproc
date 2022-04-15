
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

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdbool.h>
#include <stddef.h>  // size_t

typedef struct slist_item
{
  struct slist_item *next;
} slist_item_t;

typedef struct slist
{
  slist_item_t *head;
} slist_t;

typedef size_t hash_t;

typedef void ( *func_clear ) ( void * );
typedef bool ( *func_compare ) ( const void *key1, const void *key2 );
typedef hash_t ( *func_hash ) ( const void *data );

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

/* all function that return a pointer, return NULL on error */

hashtable_t *
hashtable_new ( func_hash fhash, func_compare fcompare, func_clear fclear );

/* return pointer value for convenience on sucess */
void *
hashtable_set ( hashtable_t *ht, const void *key, void *value );

void *
hashtable_get ( hashtable_t *ht, const void *key );

typedef int ( *hashtable_foreach_func ) ( hashtable_t *ht,
                                          void *value,
                                          void *user_data );

/* to each entries in hashtable, the function 'func' is called
    and passes as argument the entrie and 'user_data',
    if 'func' return different of zero hashtable_foreach stop
    and return the value returned from 'func' */
int
hashtable_foreach ( hashtable_t *ht,
                    hashtable_foreach_func func,
                    void *user_data );

/* remove entry from hashtable and return a pointer to entry,
  the entry needs to be handled by the user's skin yet (e.g free if
  necessary )
*/
void *
hashtable_remove ( hashtable_t *ht, const void *key );

/* equal hashtable_remove but not resize hashtable */
void *
hashtable_simple_remove ( hashtable_t *ht, const void *key );

void
hashtable_destroy ( hashtable_t *ht );

/* simple hashtable */

typedef struct hashtable_min
{
  size_t nentries;  // Total number of entries in the table
  size_t nbuckets;
  slist_t *buckets;
} hashtable_min;

hashtable_min *
hashtable_min_new ( void );

void *
hashtable_min_set ( hashtable_min *ht,
                    void *value,
                    const void *key,
                    const hash_t hash );

void *
hashtable_min_get ( hashtable_min *ht,
                    const void *key,
                    hash_t hash,
                    func_compare cmp );

int
hashtable_min_foreach ( hashtable_min *ht,
                        hashtable_foreach_func func,
                        void *user_data );

int
hashtable_min_foreach_remove ( hashtable_min *restrict ht,
                               hashtable_foreach_func func,
                               void *user_data );

void *
hashtable_min_remove ( hashtable_t *ht,
                       const void *key,
                       hash_t hash,
                       func_compare cmp );

void
hashtable_min_detroy ( hashtable_min *ht, func_clear fclear );
#endif  // HASHTABLE_H
