
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

#ifndef LIST_H
#define LIST_H

struct list_node
{
  void *data;

  struct list_node *next;
  struct list_node *prev;
};

struct list
{
  struct list_node *head;
  struct list_node *tail;
  unsigned int size;
};

struct list_node *
list_push ( struct list *list, void *data );

void
list_delete ( struct list *list, struct list_node *node );

#endif  // LIST_H
