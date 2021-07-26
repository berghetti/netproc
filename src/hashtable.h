
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

typedef size_t hash_t;

typedef struct slist_item
{
  struct slist_item *next;
} slist_item_t;

typedef struct slist
{
  slist_item_t *head;
} slist_t;

typedef void ( *fclear ) ( void * );

typedef struct hashtable
{
  size_t nentries;  // Total number of entries in the table
  size_t nbuckets;
  slist_t *buckets;

  fclear clear;  // callback clear data from user
} hashtable_t;

typedef struct hashtable_entry
{
  // used by hashtable_t.buckets to link entries
  slist_item_t _slist_item;  // "class parent"

  hash_t key_hash;
  size_t key;
  void *value;
} hashtable_entry_t;

typedef int ( *hashtable_foreach_func ) ( hashtable_t *ht,
                                          void *value,
                                          void *user_data );

/* all function that return a pointer, return NULL on error */

hashtable_t *
hashtable_new ( fclear clear );

/* return pointer value for convenience on sucess*/
void *
hashtable_set ( hashtable_t *ht, const size_t key, void *value );

void *
hashtable_get ( hashtable_t *ht, const size_t key );

int
hashtable_foreach ( hashtable_t *ht,
                    hashtable_foreach_func func,
                    void *user_data );

void *
hashtable_remove ( hashtable_t *ht, const size_t key );

void
hashtable_destroy ( hashtable_t *ht );

#endif  // HASHTABLE_H
