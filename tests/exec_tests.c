
#include "unity.h"

#include "test_hashtable.c"
#include "test_full_read.c"

void setUp(void) { /* set stuff up here */ }
void tearDown(void) { /*  clean stuff up here */ }

int main( void )
{
    UNITY_BEGIN();
    RUN_TEST(test_hashtable);
    RUN_TEST(test_full_read);

    return UNITY_END();
}
