#ifndef HUMAN_READABLE_H
#define HUMAN_READABLE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

bool
human_readable ( char *buffer, const size_t len_buff, uint64_t bytes );

#endif  // HUMAN_READABLE_H
