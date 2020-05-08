#ifndef HUMAN_READABLE_H
#define HUMAN_READABLE_H

#include <stdbool.h>  // bool type
#include <stdint.h>   // uint*_t type
#include <stdlib.h>   // size_t type

bool
human_readable ( char *buffer, const size_t len_buff, const uint64_t bytes );

#endif  // HUMAN_READABLE_H
