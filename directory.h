#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <stdint.h>
#include <stdlib.h>

int
get_numeric_directory ( uint32_t *restrict buffer,
                        const size_t lenght,
                        const char *restrict path_dir );

#endif  // DIRECTORY_H
