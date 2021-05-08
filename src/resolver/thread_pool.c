
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>  // for malloc
#include <pthread.h>
#include <limits.h>  // for PTHREAD_STACK_MIN

#include "queue.h"

#define DEFAULT_NUM_WORKERS 3

struct task
{
  void ( *func ) ( void * );  // function pointer
  void *args;                 // arg to function
};

static unsigned int task_count = 0;

static pthread_mutex_t mutex_queue = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_queue = PTHREAD_COND_INITIALIZER;

static struct task *
create_task ( void ( *func ) ( void * ), void *args )
{
  struct task *task = malloc ( sizeof ( *task ) );
  if ( !task )
    return NULL;

  task->func = func;
  task->args = args;

  return task;
}

static void
free_task ( struct task *t )
{
  free ( t );
}

static void
execute_task ( struct task *task )
{
  task->func ( task->args );
}

static void *
th_worker ( __attribute__ ( ( unused ) ) void *args )
{
  struct task *task;
  while ( 1 )
    {
      pthread_mutex_lock ( &mutex_queue );

      // wait for jobs
      while ( !task_count )
        pthread_cond_wait ( &cond_queue, &mutex_queue );

      // get first job from queue(removes it from queue)
      task = dequeue ();
      task_count--;

      pthread_mutex_unlock ( &mutex_queue );

      execute_task ( task );
      free_task ( task );
    }

  pthread_exit ( NULL );
}

int
thpool_init ( unsigned int num_workers )
{
  num_workers = ( num_workers > 0 ) ? num_workers : DEFAULT_NUM_WORKERS;

  pthread_t tid;
  pthread_attr_t attr;

  pthread_attr_init ( &attr );
  pthread_attr_setstacksize ( &attr, PTHREAD_STACK_MIN );
  pthread_attr_setdetachstate ( &attr, PTHREAD_CREATE_DETACHED );

  while ( num_workers-- )
    {
      if ( pthread_create ( &tid, &attr, th_worker, NULL ) )
        return 0;
    }

  pthread_attr_destroy ( &attr );

  return 1;
}

int
add_task ( void ( *func ) ( void * ), void *args )
{
  struct task *task = create_task ( func, args );
  if ( !task )
    return -1;

  pthread_mutex_lock ( &mutex_queue );

  // queues a job
  if ( -1 == enqueue ( task ) )
    {
      free_task ( task );
      pthread_mutex_unlock ( &mutex_queue );
      return -1;
    }

  task_count++;
  pthread_mutex_unlock ( &mutex_queue );

  // wake up a thread to work
  pthread_cond_signal ( &cond_queue );

  return 0;
}
