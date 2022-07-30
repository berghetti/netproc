
#include <stddef.h>

#include "hashtable.h"
#include "unity.h"
#include "../src/macro_util.h"

static int
cb_func ( hashtable_t *ht, void *value, void *user_data )
{
  // ugly
  ht = ht;
  value = value;

  ( *( int * ) user_data )++;

  return 0;
}

#define FROM_PTR( p ) ( ( uintptr_t ) p )
#define TO_PTR( v ) ( ( void * ) ( uintptr_t ) v )

static bool
cb_compare ( const void *key1, const void *key2 )
{
  return ( key1 == key2 );
}

static hash_t
cb_hash ( const void *key )
{
  int n = ( int ) FROM_PTR ( key );

  return n;
}

void
test1 ( void )
{
  hashtable_t *ht = hashtable_new ( cb_hash, cb_compare, NULL );

  TEST_ASSERT_NOT_NULL ( ht );
  TEST_ASSERT_EQUAL_INT ( 0, hashtable_get_nentries ( ht ) );
  TEST_ASSERT_GREATER_THAN ( 0, hashtable_get_size ( ht ) );

  TEST_ASSERT_NULL ( hashtable_get ( ht, 0 ) );

  int values[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 0 };
  int *p;

  size_t i;
  for ( i = 0; i < ARRAY_SIZE ( values ); i++ )
    {
      p = hashtable_set ( ht, TO_PTR ( values[i] ), &values[i] );
      TEST_ASSERT_EQUAL_INT ( values[i], *p );
    }

  TEST_ASSERT_EQUAL_INT ( ARRAY_SIZE ( values ),
                          hashtable_get_nentries ( ht ) );
  TEST_ASSERT_GREATER_THAN ( hashtable_get_nentries ( ht ),
                             hashtable_get_size ( ht ) );

  for ( i = 0; i < ARRAY_SIZE ( values ); i++ )
    {
      p = hashtable_get ( ht, TO_PTR ( values[i] ) );
      TEST_ASSERT_EQUAL_INT ( *p, values[i] );
    }

  p = hashtable_remove ( ht, TO_PTR ( values[0] ) );
  TEST_ASSERT_EQUAL_INT ( *p, values[0] );
  TEST_ASSERT_EQUAL_INT ( ARRAY_SIZE ( values ) - 1,
                          hashtable_get_nentries ( ht ) );
  TEST_ASSERT_NULL ( hashtable_remove ( ht, TO_PTR ( values[0] ) ) );

  int count = 0;
  TEST_ASSERT_EQUAL_INT ( 0, hashtable_foreach ( ht, cb_func, &count ) );
  TEST_ASSERT_EQUAL_INT ( ARRAY_SIZE ( values ) - 1, count );
  TEST_ASSERT_EQUAL_INT ( count, hashtable_get_nentries ( ht ) );

  hashtable_destroy ( ht );
}

static bool
cb_compare2 ( const void *key1, const void *key2 )
{
  return ( *( int * ) key1 == *( int * ) key2 );
}

static hash_t
cb_hash2 ( const void *key )
{
  return *( int * ) key;
}

static int
remove_foreach ( UNUSED hashtable_t *ht,
                 UNUSED void *value,
                 UNUSED void *user_data )
{
  return 1;
}

static void
test_hashtable_foreach_safe ( void )
{
  hashtable_t *ht = hashtable_new ( cb_hash2, cb_compare2, NULL );

  int values[] = { 10, 20,  30,  40,  50,  60,  70, 80,
                   90, 100, 110, 120, 130, 140, 150 };
  int *p;

  size_t i;
  for ( i = 0; i < ARRAY_SIZE ( values ); i++ )
    {
      p = hashtable_set ( ht, &values[i], &values[i] );
      TEST_ASSERT_EQUAL_INT ( values[i], *p );
    }

  hashtable_foreach_remove ( ht, remove_foreach, NULL );

  TEST_ASSERT_EQUAL_INT ( 0, hashtable_get_nentries ( ht ) );

  hashtable_destroy ( ht );
}

void
test_hashtable ( void )
{
  test1 ();
  test_hashtable_foreach_safe ();
}
