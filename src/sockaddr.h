#ifndef SOCKADDR_H
#define SOCKADDR_H

#include <stdint.h>
#include <sys/socket.h>  // struct sockaddr
#include <netinet/in.h>  // struct sockaddr_in sockaddr_in6

// information network layer
union inet_all
{
  uint32_t all[4];
  uint32_t ip;
  uint32_t ip6[4];
  struct in_addr in;
  struct in6_addr in6;
};

struct layer3
{
  union inet_all local;
  union inet_all remote;
  // uint16_t l3_proto; // ipv4 or ipv6
};

// information transport layer (only port)
struct layer4
{
  uint16_t local_port;
  uint16_t remote_port;
  uint8_t protocol;  // l4 protocol number (e.g 6 to TCP or 17 to UDP)
};

// typle identify a connection ip:port <-> ip:port
struct tuple
{
  struct layer3 l3;
  struct layer4 l4;
};

union sockaddr_all
{
  struct sockaddr sa;
  struct sockaddr_in in;
  struct sockaddr_in6 in6;
};

#endif  // SOCKADDR_H
