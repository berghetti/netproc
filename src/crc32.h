#ifndef CRC32_H_
#define CRC32_H_

#include <inttypes.h>

#define CRC_BUFFER_SIZE 8192

typedef uint64_t hash_t;

hash_t
get_crc32_file (const char *file_path);


#endif
