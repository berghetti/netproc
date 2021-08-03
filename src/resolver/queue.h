
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

struct queue_node
{
  void *data;
  struct queue_node *next;
};

typedef void ( *fclear ) ( void * );

struct queue
{
  fclear clear;  // callback user init
  struct queue_node *head;
  struct queue_node *tail;
  unsigned int size;
};

struct queue *
queue_new ( fclear clear );

struct queue_node *
enqueue ( struct queue *queue, void *data );

void *
dequeue ( struct queue *queue );

void
queue_destroy ( struct queue *queue );

#endif  // QUEUE_H
