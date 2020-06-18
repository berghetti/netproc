#ifndef CRC32_H_
#define CRC32_H_

#include <inttypes.h>

typedef uint64_t hash_t;

// hash_t
// get_crc32_file (const char *file_path);

hash_t
get_crc32_udp_conection ( void );

#endif
