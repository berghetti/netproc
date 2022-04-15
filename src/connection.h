
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

#ifndef CONECTION_H
#define CONECTION_H

#include <stdbool.h>
#include <stdint.h>

#include "rate.h"  // struct net_stat
#include "sockaddr.h"

// forward declaration
typedef struct process process_t;

/* stores the information exported by the kernel in /proc/net/tcp | udp.
   each conn has two entries in hashtable, one with key inode and other with key
   tuple */
typedef struct conection
{
  struct net_stat net_stat;  // assign in statistics.c
  struct tuple tuple;        // layer 3 and 4 info
  process_t *proc;           // process the connection belongs to
  unsigned long inode;       // kernel linux usage this type to inode
  int if_index;              // assign in statistics.c
  uint8_t state;             // status tcp connection

  // internal state
  uint8_t refs_active;  // if 1 connection is removed from ht, if 0 connection
                        // is removed from ht and free
  uint8_t refs_exit;    // usage to cleanup hashtable
} connection_t;

bool
connection_init ( void );

bool
connection_update ( const int proto );

connection_t *
connection_get_by_inode ( const unsigned long inode );

connection_t *
connection_get_by_tuple ( struct tuple *tuple );

void
connection_free ( void );

#endif  // CONECTION_H
