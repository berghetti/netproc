
#include <string.h>

#include "str.h"
#include "unity.h"

void
test_str ()
{
  // 300 'A' + 1 'space' + 2 'B'...
  const char *str =
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
          "AAAAAAAAAA BB CCC DDDD E";

  TEST_ASSERT_EQUAL_INT ( 300, strlen_space ( str ) );

  TEST_ASSERT_EQUAL_INT ( 302, find_last_char ( str, strlen ( str ), 'B' ) );
  TEST_ASSERT_EQUAL_INT ( 306, find_last_char ( str, strlen ( str ), 'C' ) );
  TEST_ASSERT_EQUAL_INT ( 'E',
                          str[find_last_char ( str, strlen ( str ), 'E' )] );

  TEST_ASSERT_EQUAL_INT ( -1, find_last_char ( str, strlen ( str ), 'G' ) );
}
