
#include <stddef.h>

#include "hashtable.h"
#include "unity.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

static int
cb_func( hashtable_t *ht, void *value, void *user_data )
{
  //ugly
  ht = ht;
  value = value;

  (*(int *)user_data)++;

  return 0;
}

#define ARRAY_SIZE(a) ( sizeof( a ) / sizeof(a[0]) )

void test_hashtable( void )
{
  hashtable_t *ht = hashtable_new( NULL );

  TEST_ASSERT_NOT_NULL(ht);
  TEST_ASSERT_EQUAL_INT(0, ht->nentries);
  TEST_ASSERT_GREATER_THAN(0, ht->nbuckets);

  TEST_ASSERT_NULL( hashtable_get( ht, 0 ) );

  int values[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 0 };
  int *p;

  for (int i = 0; i < ARRAY_SIZE(values); i++ )
    {
      p = hashtable_set( ht, values[i], &values[i] );
      TEST_ASSERT_EQUAL_INT ( values[i], *p );
    }

  TEST_ASSERT_EQUAL_INT( ARRAY_SIZE(values), ht->nentries );
  TEST_ASSERT_GREATER_THAN( ht->nentries, ht->nbuckets );

  for (int i = 0; i < ARRAY_SIZE(values); i++ )
    {
      p = hashtable_get( ht, values[i] );
      TEST_ASSERT_EQUAL_INT( *p, values[i] );
    }

  p = hashtable_remove( ht, values[0] );
  TEST_ASSERT_EQUAL_INT(*p, values[0]);
  TEST_ASSERT_EQUAL_INT( ARRAY_SIZE(values) - 1, ht->nentries);
  TEST_ASSERT_NULL(hashtable_remove( ht, values[0] ) );

  int count = 0;
  TEST_ASSERT_EQUAL_INT(0, hashtable_foreach(ht, cb_func, &count) );
  TEST_ASSERT_EQUAL_INT( ARRAY_SIZE(values) - 1, count);
  TEST_ASSERT_EQUAL_INT( count, ht->nentries );

  hashtable_destroy( ht );
}

int main( void )
{
    UNITY_BEGIN();
    RUN_TEST(test_hashtable);

    return UNITY_END();
}
