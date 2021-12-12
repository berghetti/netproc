
/*
 *  Copyright (C) 2021 Mayco S. Berghetti
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

#include <stdlib.h>  // malloc
#include "queue.h"

struct queue_node
{
  struct queue_node *next;
  void *data;
};

struct queue
{
  fclear clear;  // callback user init
  struct queue_node *head;
  struct queue_node *tail;
  unsigned int size;
};

static struct queue_node *
create_element ( void *data )
{
  struct queue_node *e = malloc ( sizeof ( *e ) );

  if ( e )
    {
      e->data = data;
      e->next = NULL;
    }

  return e;
}

struct queue *
queue_new ( fclear clear )
{
  struct queue *q = malloc ( sizeof *q );

  if ( q )
    {
      q->clear = clear;
      q->size = 0;
    }

  return q;
}

int
enqueue ( struct queue *restrict queue, void *restrict data )
{
  struct queue_node *element = create_element ( data );

  if ( !element )
    return 0;

  if ( !queue->size )
    queue->head = element;
  else
    queue->tail->next = element;

  queue->tail = element;

  queue->size++;
  return queue->size;
}

void *
dequeue ( struct queue *queue )
{
  // empty queue
  if ( !queue->size )
    return NULL;

  queue->size--;
  void *data = queue->head->data;
  struct queue_node *tmp = queue->head->next;

  free ( queue->head );
  queue->head = tmp;

  return data;
}

void
queue_destroy ( struct queue *queue )
{
  while ( queue->size )
    {
      void *data = dequeue ( queue );

      if ( queue->clear )
        queue->clear ( data );
    }

  free ( queue );
}

unsigned int
get_queue_size ( struct queue *queue )
{
  return queue->size;
}
