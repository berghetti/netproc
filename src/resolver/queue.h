
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QUEUE_H
#define QUEUE_H

// Forward declaration
struct queue;

typedef void ( *fclear ) ( void * );

struct queue *
queue_new ( fclear clear );

// returns 0 on failure and queue length on success
int
enqueue ( struct queue *queue, void *data );

// return NULL on failure
void *
dequeue ( struct queue *queue );

void
queue_destroy ( struct queue *queue );

unsigned int
get_queue_size ( struct queue *queue );

#endif  // QUEUE_H
