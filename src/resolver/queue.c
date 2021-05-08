
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

#include <stdlib.h>  // for malloc

struct queue
{
  void *data;
  struct queue *next;
};

static struct queue *queue_head = NULL;
static struct queue *queue_tail = NULL;

static struct queue *
create_element ( void *data )
{
  struct queue *e = malloc ( sizeof ( *e ) );
  if ( !e )
    return NULL;

  e->data = data;
  e->next = NULL;

  return e;
}

int
enqueue ( void *data )
{
  struct queue *element = create_element ( data );
  if ( !element )
    return -1;

  if ( !queue_head )
    {
      queue_head = element;
      queue_tail = element;
    }
  else
    {
      queue_tail->next = element;
      queue_tail = element;
    }

  return 0;
}

void *
dequeue ( void )
{
  void *data = queue_head->data;

  void *t = queue_head;
  queue_head = queue_head->next;
  free ( t );

  return data;
}
