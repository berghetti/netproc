#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

struct config_op
{
  char *iface;             // bind interface
  int *color_scheme;       // scheme colors 
  bool udp;                // TCP or UDP
  bool view_si;            // SI or IEC prefix
  bool view_bytes;         // view in bytes or bits
  bool view_conections;    // show conections each process
  bool translate_host;     // translate ip to name using DNS
  bool translate_service;  // translate port to service
  uint8_t tic_tac;         // sinc program
};

struct config_op *
parse_options ( int argc, const char **argv );

#endif  // CONFIG_H
