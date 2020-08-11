#ifndef SOCK_UTIL_H
#define SOCK_UTIL_H

#include <stdbool.h>
#include <sys/socket.h>

bool
check_addr_equal ( struct sockaddr_storage *addr1,
                   struct sockaddr_storage *addr2 );

// transform binary to text
int
sockaddr_ntop ( struct sockaddr_storage *addr,
                char *buf,
                const size_t len_buff );

#endif  // SOCK_UTIL_H
