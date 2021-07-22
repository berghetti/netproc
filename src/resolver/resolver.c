
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

#include "thread_pool.h"
#include "domain.h"

int
resolver_init ( unsigned int cache_size, unsigned int num_workers )
{
  if ( !thpool_init ( num_workers ) )
    return 0;

  cache_domain_init ( cache_size );

  return 1;
}

void
resolver_free ( void )
{
  thpool_free ();
  cache_domain_free ();
}
