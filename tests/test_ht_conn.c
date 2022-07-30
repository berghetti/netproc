
#include "unity/unity.h"

#include "../src/connection.c"

// #include "../src/m_error.h"
//
#include <stdlib.h>
#include <time.h>

static connection_t *
create_conn_fake ( void )
{
  connection_t *conn = calloc ( 1, sizeof *conn );

  if ( conn )
    {
      conn->inode = rand ();
      conn->tuple.l3.local.ip = rand ();
      conn->tuple.l3.remote.ip = rand ();
      conn->tuple.l4.local_port = rand () % 0xffff;
      conn->tuple.l4.remote_port = rand () % 0xffff;
      conn->refs_active = 3;
    }

  return conn;
}

#define NUM_CONN 16

static void
test_insert ( void )
{
  for ( int i = 0; i < NUM_CONN; i++ )
    {
      connection_t *conn = create_conn_fake ();
      TEST_ASSERT_NOT_NULL ( conn );

      connection_insert ( conn );
      TEST_ASSERT_EQUAL_INT ( 2 * ( i + 1 ),
                              hashtable_get_nentries ( ht_connections ) );
    }
}

static void
test_search ( void )
{
  connection_t *conn = create_conn_fake ();
  TEST_ASSERT_NOT_NULL ( conn );

  TEST_ASSERT_NULL ( connection_get_by_inode ( conn->inode ) );
  TEST_ASSERT_NULL ( connection_get_by_tuple ( &conn->tuple ) );

  connection_insert ( conn );

  TEST_ASSERT_EQUAL ( 2 * ( NUM_CONN + 1 ),
                      hashtable_get_nentries ( ht_connections ) );

  connection_t *tmp;

  tmp = connection_get_by_inode ( conn->inode );
  TEST_ASSERT_EQUAL_PTR ( conn, tmp );

  tmp = connection_get_by_tuple ( &conn->tuple );
  TEST_ASSERT_EQUAL_PTR ( conn, tmp );
}

static void
test_delete ( void )
{
  hashtable_foreach_remove ( ht_connections, remove_inactives_conns, NULL );
  TEST_ASSERT_EQUAL ( NUM_CONN * 2 + 2,
                      hashtable_get_nentries ( ht_connections ) );

  // removed conns only next update
  hashtable_foreach_remove ( ht_connections, remove_inactives_conns, NULL );
  TEST_ASSERT_EQUAL ( 0, hashtable_get_nentries ( ht_connections ) );
}

static void
test_conn_update ( void )
{
  TEST_ASSERT_TRUE ( connection_update ( TCP | UDP ) );
}

void
test_ht_conn ( void )
{
  srand ( time ( NULL ) );

  TEST_ASSERT_TRUE ( connection_init () );

  test_insert ();
  test_search ();
  test_delete ();

  test_conn_update ();
  size_t hold = hashtable_get_nentries ( ht_connections );
  hashtable_foreach_remove ( ht_connections, remove_inactives_conns, NULL );
  TEST_ASSERT_EQUAL ( hold, hashtable_get_nentries ( ht_connections ) );

  hashtable_foreach_remove ( ht_connections, remove_inactives_conns, NULL );
  TEST_ASSERT_EQUAL ( 0, hashtable_get_nentries ( ht_connections ) );

  connection_free ();
}
