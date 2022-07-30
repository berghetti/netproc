
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

#ifndef SLIST_H
#define SLIST_H

typedef struct slist_item
{
  struct slist_item *next;
} slist_item_t;

typedef struct slist
{
  slist_item_t *head;
} slist_t;

static inline void
slist_preprend ( slist_t *list, slist_item_t *item )
{
  item->next = list->head;
  list->head = item;
}

static void
slist_remove ( slist_t *list, slist_item_t *previous, slist_item_t *item )
{
  if ( previous )
    previous->next = item->next;
  else
    list->head = item->next;
}

#endif  // SLIST_H
