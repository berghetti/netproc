
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
#include <stdbool.h>
#include <limits.h>  // for PTHREAD_STACK_MIN
#include <unistd.h>  // sleep
#include <pthread.h>

#include "queue.h"
#include "get_cpu.h"

#define DEFAULT_NUM_WORKERS 3

struct bsem
{
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  bool value;
};

struct task
{
  void ( *func ) ( void * );  // function pointer
  void *args;                 // arg to function
};

static struct bsem bsem_jobs = { .mutex = PTHREAD_MUTEX_INITIALIZER,
                                 .cond = PTHREAD_COND_INITIALIZER,
                                 .value = false };

static struct bsem bsem_exit = { .mutex = PTHREAD_MUTEX_INITIALIZER,
                                 .cond = PTHREAD_COND_INITIALIZER,
                                 .value = false };

static volatile bool worker_stop = false;

static volatile unsigned int workers_alive = 0;
static pthread_mutex_t mutex_workers_alive = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_queue = PTHREAD_MUTEX_INITIALIZER;

static struct queue *queue_task;

static void
bsem_post ( struct bsem *sem )
{
  pthread_mutex_lock ( &( sem->mutex ) );
  sem->value = true;
  pthread_cond_signal ( &sem->cond );
  pthread_mutex_unlock ( &( sem->mutex ) );
}

static void
bsem_post_all ( struct bsem *sem )
{
  pthread_mutex_lock ( &( sem->mutex ) );
  sem->value = true;
  pthread_cond_broadcast ( &( sem->cond ) );
  pthread_mutex_unlock ( &( sem->mutex ) );
}

static void
bsem_wait ( struct bsem *sem )
{
  pthread_mutex_lock ( &( sem->mutex ) );
  while ( !sem->value )
    pthread_cond_wait ( &sem->cond, &( sem->mutex ) );

  sem->value = false;
  pthread_mutex_unlock ( &( sem->mutex ) );
}

static struct task *
create_task ( void ( *func ) ( void * ), void *args )
{
  struct task *task = malloc ( sizeof ( *task ) );

  if ( task )
    {
      task->func = func;
      task->args = args;
    }

  return task;
}

static void
free_task ( struct task *task )
{
  free ( task );
}

static void
execute_task ( struct task *task )
{
  task->func ( task->args );
}

static void *
th_worker ( __attribute__ ( ( unused ) ) void *args )
{
  pthread_mutex_lock ( &mutex_workers_alive );
  workers_alive++;
  pthread_mutex_unlock ( &mutex_workers_alive );

  while ( !worker_stop )
    {
      // wait for jobs
      bsem_wait ( &bsem_jobs );

      if ( worker_stop )
        break;

      pthread_mutex_lock ( &mutex_queue );
      struct task *task = dequeue ( queue_task );
      if ( queue_task->size )
        bsem_post ( &bsem_jobs );  // rearm other thread
      pthread_mutex_unlock ( &mutex_queue );

      if ( task )
        {
          execute_task ( task );
          free_task ( task );
        }
    }

  pthread_mutex_lock ( &mutex_workers_alive );
  workers_alive--;
  pthread_mutex_unlock ( &mutex_workers_alive );

  // make up main thread to exit
  bsem_post ( &bsem_exit );

  pthread_exit ( NULL );
}

int
thpool_init ( unsigned int num_workers )
{
  queue_task = queue_new ( free );
  if ( !queue_task )
    return 0;

  if ( !num_workers && !( num_workers = get_count_cpu () - 1 ) )
    num_workers = DEFAULT_NUM_WORKERS;

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
    return 0;

  // push job in queue
  pthread_mutex_lock ( &mutex_queue );
  if ( !enqueue ( queue_task, task ) )
    {
      free_task ( task );
      pthread_mutex_unlock ( &mutex_queue );
      return 0;
    }

  // wake up a thread to work
  bsem_post ( &bsem_jobs );
  pthread_mutex_unlock ( &mutex_queue );

  return 1;
}

void
thpool_free ( void )
{
  worker_stop = true;

  int wait = DEFAULT_NUM_WORKERS;
  while ( workers_alive && wait-- )
    {
      bsem_post_all ( &bsem_jobs );
      bsem_wait ( &bsem_exit );
    }

  pthread_mutex_lock ( &mutex_queue );
  queue_destroy ( queue_task );
  pthread_mutex_unlock ( &mutex_queue );
}
