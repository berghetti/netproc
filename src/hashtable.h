
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

typedef size_t hash_t;
typedef struct hashtable hashtable_t;

typedef void ( *func_clear ) ( void * );
typedef bool ( *func_compare ) ( const void *key1, const void *key2 );
typedef hash_t ( *func_hash ) ( const void *data );
typedef int ( *hashtable_foreach_func ) ( hashtable_t *ht,
                                          void *value,
                                          void *user_data );

/* all function that return a pointer, return NULL on error,
  some functions has two version as 'hashtable_set' and 'hashtable_min_set'
  the version with 'min' in name is used internament and can be used by user to
  a more generic use */

hashtable_t *
hashtable_min_new ( void );

hashtable_t *
hashtable_new ( func_hash fhash, func_compare fcompare, func_clear fclear );

/* return pointer value for convenience on sucess */
void *
hashtable_min_set ( hashtable_t *ht,
                    void *value,
                    const void *key,
                    const hash_t hash );

void *
hashtable_set ( hashtable_t *ht, const void *key, void *value );

void *
hashtable_min_get ( hashtable_t *ht,
                    const void *key,
                    hash_t hash,
                    func_compare cmp );

void *
hashtable_get ( hashtable_t *ht, const void *key );

size_t
hashtable_get_nentries ( hashtable_t *ht );

size_t
hashtable_get_size ( hashtable_t *ht );

/* to each entries in hashtable, the function 'func' is called
    and passes as argument the entrie and 'user_data',
    if 'func' return different of zero hashtable_foreach stop
    and return the value returned from 'func' */
int
hashtable_foreach ( hashtable_t *ht,
                    hashtable_foreach_func func,
                    void *user_data );

/* to each entries in hashtable, the function 'to_remove' is called
    and passes as argument the entrie and 'user_data',
    if 'to_remove' return true the entry be removed from ht */
void
hashtable_foreach_remove ( hashtable_t *ht,
                           hashtable_foreach_func to_remove,
                           void *user_data );

/* remove entry from hashtable and return a pointer to entry,
  the entry needs to be handled by the user's skin yet (e.g free if
  necessary )
*/
void *
hashtable_min_remove ( hashtable_t *ht,
                       const void *key,
                       hash_t hash,
                       func_compare cmp );

void *
hashtable_remove ( hashtable_t *ht, const void *key );

void
hashtable_min_detroy ( hashtable_t *ht, func_clear fclear );

void
hashtable_destroy ( hashtable_t *ht );

#endif  // HASHTABLE_H
