
/*
 *  Copyright (C) 2020-2022 Mayco S. Berghetti
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

#ifndef SOCK_UTIL_H
#define SOCK_UTIL_H

#include <stdbool.h>
#include <sys/socket.h>

#include "../sockaddr.h"

// return 1 if ip address is equal or 0
bool
check_addr_equal ( union sockaddr_all *addr1, union sockaddr_all *addr2 );

/* transform binary ip to text
   only to AF_INET and AF_INET6,
  len_buff must be enough to store a ip address of correspodent family */
void
sockaddr_ntop ( union sockaddr_all *addr, char *buf, const size_t len_buff );

#endif  // SOCK_UTIL_H
