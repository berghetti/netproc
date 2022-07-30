
#include <stdio.h>
#include <string.h>

#include "str.h"
#include "unity.h"

void
test_str ()
{
  // 300 'A' + 1 'space' + 2 'B'...
  const char *str = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                    "AAAAAAAAAA BB CCC DDDD E";

  TEST_ASSERT_EQUAL_INT ( 300, strlen_space ( str ) );

  TEST_ASSERT_EQUAL_INT ( 302, index_last_char ( str, 'B' ) );
  TEST_ASSERT_EQUAL_INT ( 306, index_last_char ( str, 'C' ) );

  TEST_ASSERT_EQUAL_INT ( 'E', str[index_last_char ( str, 'E' )] );

  TEST_ASSERT_EQUAL_INT ( -1, index_last_char ( str, 'G' ) );

  const char *str2 = "/usr/sbin/ntpd -p /var/run/ntpd.pid -g -u 123:134";
  TEST_ASSERT_EQUAL_INT ( 14, strlen_space ( str2 ) );
}
