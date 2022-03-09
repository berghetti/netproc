
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "unity.h"

#include "jhash.h"
#include "connection.h"
#include "macro_util.h"

// values to key_type
#define KEY_INODE 1
#define KEY_TUPLE 2

static int key_type;

static hashtable_t *ht_connections;

static inline void
connection_insert_by_inode ( connection_t *conn )
{
  key_type = KEY_INODE;
  hashtable_set ( ht_connections, &conn->inode, conn );
}

static inline void
connection_insert_by_tuple ( connection_t *conn )
{
  key_type = KEY_TUPLE;
  hashtable_set ( ht_connections, &conn->tuple, conn );
}

static void
connection_insert ( connection_t *conn )
{
  conn->use = 2;  // tow references to conn on hashtable

  // make two entris in hashtable to same connection
  connection_insert_by_inode ( conn );
  connection_insert_by_tuple ( conn );
}

static inline void
connection_remove_by_inode ( connection_t *conn )
{
  key_type = KEY_INODE;
  hashtable_remove ( ht_connections, &conn->inode );
}

static inline void
connection_remove_by_tuple ( connection_t *conn )
{
  key_type = KEY_TUPLE;
  hashtable_remove ( ht_connections, &conn->tuple );
}

static void
connection_remove ( connection_t *conn )
{
  connection_remove_by_inode ( conn );
  connection_remove_by_tuple ( conn );
  free ( conn );
}

connection_t *
connection_get_by_inode ( const unsigned long inode )
{
  key_type = KEY_INODE;
  return hashtable_get ( ht_connections, &inode );
}

connection_t *
connection_get_by_tuple ( struct tuple *tuple )
{
  key_type = KEY_TUPLE;
  return hashtable_get ( ht_connections, tuple );
}

static hash_t
ht_cb_hash ( const void *key )
{
  size_t size;
  switch ( key_type )
    {
      case KEY_INODE:
        size = SIZEOF_MEMBER ( connection_t, inode );
        break;
      case KEY_TUPLE:
        size = SIZEOF_MEMBER ( connection_t, tuple );
        break;
      default:
        size = 0;
    }

  return jhash8 ( key, size, 0 );
}

static int
ht_cb_compare ( const void *key1, const void *key2 )
{
  switch ( key_type )
    {
      case KEY_INODE:
        return ( *( unsigned long * ) key1 == *( unsigned long * ) key2 );
      case KEY_TUPLE:
        // print_tuple ( ( struct tuple * ) key1, ( struct tuple * ) key2 );
        return ( 0 == memcmp ( key1, key2, sizeof ( struct tuple ) ) );
    }

  return 0;
}

static void
ht_cb_free ( void *arg )
{
  connection_t *conn = arg;

  conn->use--;

  if ( !conn->use )
    free ( arg );
}

static int
remove_dead_conn ( UNUSED hashtable_t *ht, void *value, UNUSED void *user_data )
{
  connection_t *conn = value;

  if ( !conn->active )
    {
      conn->use--;
      if ( !conn->use )
        connection_remove ( conn );
    }
  else
    conn->active = false;

  return 0;
}

void
test_hash_conn ( void )
{
  ht_connections = hashtable_new ( ht_cb_hash, ht_cb_compare, ht_cb_free );

  TEST_ASSERT_NOT_NULL ( ht_connections );

  connection_t *conn = calloc ( 1, sizeof *conn );
  TEST_ASSERT_NOT_NULL ( conn );

  conn->tuple.l3.local.ip = 0xAABBCCDD;
  conn->tuple.l3.remote.ip = 0xDDCCBBAA;
  conn->tuple.l4.local_port = 123;
  conn->tuple.l4.remote_port = 80;
  conn->inode = 5555;
  conn->active = 1;

  connection_insert ( conn );
  TEST_ASSERT_EQUAL_INT ( 2, ht_connections->nentries );

  connection_t *cp;
  cp = connection_get_by_inode ( conn->inode );
  TEST_ASSERT_EQUAL_PTR ( conn, cp );

  cp = connection_get_by_tuple ( &conn->tuple );
  TEST_ASSERT_EQUAL_PTR ( conn, cp );

  hashtable_foreach ( ht_connections, remove_dead_conn, NULL );

  TEST_ASSERT_EQUAL_INT ( 2, ht_connections->nentries );

  cp->active = 0;

  hashtable_foreach ( ht_connections, remove_dead_conn, NULL );
  TEST_ASSERT_EQUAL_INT ( 0, ht_connections->nentries );

  hashtable_destroy ( ht_connections );
}
