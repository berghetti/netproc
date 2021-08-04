
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

#ifndef FULL_READ_H
#define FULL_READ_H

/* read all data from file descriptor fd and alloc memory necessary
   return total bytes read or -1 on failure,
   if failure is not necessary free the buffer */
ssize_t
full_read ( const int fd, char **buffer );

#endif  // FULL_READ_H
