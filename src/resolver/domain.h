
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

#ifndef DOMAIN_H
#define DOMAIN_H

#include <sys/socket.h>  // struct sockaddr_storage
#include <netdb.h>       // NI_MAXHOST

struct host
{
  struct sockaddr_storage ss;
  char fqdn[NI_MAXHOST];
  int status;
};

// host.status
#define RESOLVED 1
#define RESOLVING 2

int
cache_domain_init ( unsigned int size );

// retorna imediatamente o ip em formato de texto, porém na proxima requisição
// irá retornar o dominio que estará em cache (se tudo der certo).
// evitando a latencia que uma consulta DNS pode ter.
int
ip2domain ( struct sockaddr_storage *ss, char *buff, const size_t buff_len );

void
cache_domain_free ( void );

#endif  // DOMAIN_H
