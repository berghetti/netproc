
#include <stdlib.h>

#include "queue.h"
#include "unity.h"

#define TOT_DATA 99

struct mydata
{
  int data;
};

static struct mydata *
create_mydata ( size_t i )
{
  struct mydata *d = malloc ( sizeof *d );
  TEST_ASSERT_NOT_NULL ( d );

  d->data = i;
  return d;
}

void
test_queue ( void )
{
  struct queue *q = queue_new ( free );
  TEST_ASSERT_NOT_NULL ( q );

  // enqueue return size of queue if sucess, this should be different of 0
  for ( int i = 0; i < TOT_DATA; i++ )
    {
      TEST_ASSERT_EQUAL_INT ( i + 1, enqueue ( q, create_mydata ( i ) ) );
    }

  TEST_ASSERT_EQUAL_INT ( TOT_DATA, get_queue_size ( q ) );

  struct mydata *d;
  for ( int i = 0; i < TOT_DATA - 5; i++ )
    {
      TEST_ASSERT_NOT_NULL ( ( d = dequeue ( q ) ) );
      TEST_ASSERT_EQUAL_INT ( i, d->data );
      free ( d );
    }

  TEST_ASSERT_EQUAL_INT ( 5, get_queue_size ( q ) );

  queue_destroy ( q );
}
