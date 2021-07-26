
#include <stdlib.h>     // free
#include <sys/types.h>  // open
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>     // close

#include "unity.h"
#include "full_read.h"

static void
test_full_read ( void )
{
  // echo -e -n '\x00\x10\x20\x30\x40\x50\x60\x70\x80\x90' > file_test_full_read.bin
  int fd = open ( "file_test_full_read.bin", O_RDONLY );
  TEST_ASSERT ( fd != -1 );

  char *buff;
  ssize_t total_read = full_read ( fd, &buff );
  TEST_ASSERT ( total_read != -1 );
  close(fd);

  TEST_ASSERT_NOT_NULL ( buff );
  TEST_ASSERT_EQUAL_INT( 10, total_read );

  int value = 0x0;
  for (int i = 0; i < total_read; i++)
    {
      TEST_ASSERT_EQUAL_UINT( value, (unsigned char) buff[i] );
      value += 0x10;
    }

  free (buff);
}
