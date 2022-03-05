
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>      // variable errno
#include <stdbool.h>    // type boolean
#include <stdio.h>      // snprintf
#include <string.h>     // memset
#include <unistd.h>     // readliink
#include <sys/types.h>  // open
#include <sys/stat.h>
#include <fcntl.h>

#include "processes.h"  // process_t
#include "jhash.h"
#include "hashtable.h"
#include "vector.h"
#include "full_read.h"
#include "config.h"
#include "m_error.h"  // ERROR_DEBUG
#include "macro_util.h"

// 4294967295
#define LEN_MAX_INT 10

// /proc/%d/cmdline
#define MAX_CMDLINE 14 + LEN_MAX_INT

// /proc/<pid>/fd/<id-fd> + 2 align
#define MAX_PATH_FD 10 + LEN_MAX_INT + LEN_MAX_INT + 2

// strlen ("socket:[4294967295]") + 5 align
#define MAX_NAME_SOCKET 9 + LEN_MAX_INT + 5

static hashtable_t *ht_process;

static void
handle_cmdline ( char *buff, size_t len )
{
  while ( --len )
    {
      if ( *buff == '\0' || *buff == '\n' )
        *buff = ' ';

      buff++;
    }

  // warranty, already null terminated
  *buff = '\0';
}

// armazena o nome do processo no buffer e retorna
// o tamanho do nome do processo ou -1 em caso de erro,
// função cuida da alocação de memoria para o nome do processo
static ssize_t
get_name_process ( char **buffer, const pid_t pid )
{
  char path_cmdline[MAX_CMDLINE];
  snprintf ( path_cmdline, sizeof ( path_cmdline ), "/proc/%d/cmdline", pid );

  int fd = open ( path_cmdline, O_RDONLY );
  if ( fd == -1 )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return -1;
    }

  ssize_t total_read = full_read ( fd, buffer );
  close ( fd );

  if ( total_read <= 0 )
    {
      ERROR_DEBUG ( "%s", "error read process name" );
      return -1;
    }

  handle_cmdline ( *buffer, ( size_t ) total_read );

  // last bytes is null
  return total_read - 1;
}

static process_t *
create_new_process ( pid_t pid )
{
  process_t *proc = malloc ( sizeof *proc );

  if ( proc )
    {
      proc->pid = pid;
      proc->active = true;

      proc->conections = vector_new ( sizeof ( connection_t * ) );
      if ( !proc->conections )
        goto ERROR;

      proc->total_conections = 0;
      if ( -1 == get_name_process ( &proc->name, pid ) )
        goto ERROR;

      memset ( &proc->net_stat, 0, sizeof ( struct net_stat ) );
    }

  return proc;

ERROR:
  free ( proc );
  return NULL;
}

static void
free_process ( void *arg )
{
  process_t *process = arg;
  free ( process->name );
  vector_free ( process->conections );
  free ( process );
}

struct my_array
{
  process_t **data;
  size_t pos;
};

/* copy hashtable to array removing processes inactives */
static int
to_array ( hashtable_t *ht, void *value, void *user_data )
{
  struct my_array *ar = user_data;
  process_t *proc = value;

  if ( !proc->active )
    free_process ( hashtable_remove ( ht, &proc->pid ) );
  else
    {
      proc->active = false;  // reset status to next update checking

      ar->data[ar->pos] = proc;
      ar->pos++;
    }

  return 0;
}

static process_t **
copy_ht_to_array ( hashtable_t *ht, process_t **proc )
{
  process_t **pp =
          realloc ( proc, ( ht->nentries + 1 ) * sizeof ( process_t * ) );

  if ( pp )
    {
      struct my_array my_array = { .data = pp, .pos = 0 };

      hashtable_foreach ( ht, to_array, &my_array );

      pp[ht->nentries] = NULL;  // last pointer
    }

  return pp;
}

static int
ht_cb_compare ( const void *key1, const void *key2 )
{
  return ( *( pid_t * ) key1 == *( pid_t * ) key2 );
}

static hash_t
ht_cb_hash ( const void *key )
{
  return jhash8 ( key, sizeof ( pid_t ), 0 );
}

struct processes *
processes_init ( void )
{
  struct processes *procs = calloc ( 1, sizeof *procs );
  if ( !procs )
    return NULL;

  ht_process = hashtable_new ( ht_cb_hash, ht_cb_compare, free_process );
  if ( !ht_process )
    {
      free ( procs );
      return NULL;
    }

  return procs;
}

/*
 percorre todos os processos encontrados no diretório '/proc/',
 em cada processo encontrado armazena todos os file descriptors
 do processo - /proc/$id/fd - no buffer fds
 depois compara o link simbolico apontado pelo FD com 'socket:[inode]',
 sendo inode coletado do arquivo '/proc/net/tcp', caso a comparação seja igual,
 encontramos o processo que corresponde ao inode (conexão).
*/
int
processes_get ( struct processes *procs, struct config_op *co )
{
  // TODO: check if type uint32_t is correct/safe
  uint32_t *pids = NULL;
  int total_process = get_numeric_directory ( &pids, "/proc/" );

  if ( -1 == total_process )
    {
      ERROR_DEBUG ( "%s", "backtrace" );
      return 0;
    }

  if ( !connection_update ( co->proto ) )
    {
      ERROR_DEBUG ( "%s", "backtrace" );
      free ( pids );
      return 0;
    }

  uint32_t *fds = NULL;
  for ( int index_pid = 0; index_pid < total_process; index_pid++ )
    {
      // TODO: create variable pid_t
      char path_fd[MAX_PATH_FD];
      int ret_sn = snprintf ( path_fd,
                              sizeof ( path_fd ),
                              "/proc/%d/fd/",
                              pids[index_pid] );

      int total_fd_process = get_numeric_directory ( &fds, path_fd );

      if ( -1 == total_fd_process )
        continue;

      process_t *proc = hashtable_get ( ht_process, &pids[index_pid] );

      if ( proc )
        {
          proc->active = 1;
          vector_clear ( proc->conections );
        }

      for ( int index_fd = 0; index_fd < total_fd_process; index_fd++ )
        {
          // concat "/proc/<pid>/fd/%d"
          snprintf ( path_fd + ret_sn,
                     sizeof ( path_fd ) - ret_sn,
                     "%d",
                     fds[index_fd] );

          char data_fd[MAX_NAME_SOCKET];
          ssize_t len_link =
                  readlink ( path_fd, data_fd, sizeof ( data_fd ) - 1 );

          if ( len_link == -1 )
            continue;

          data_fd[len_link] = '\0';

          unsigned long int inode;
          if ( 1 != sscanf ( data_fd, "socket:[%lu", &inode ) )
            continue;

          connection_t *conn = connection_get ( inode );

          if ( !conn )
            continue;

          if ( !proc )
            {
              proc = create_new_process ( pids[index_pid] );
              if ( !proc )
                break;  // no return error, check others processes

              hashtable_set ( ht_process, &proc->pid, proc );
            }

          vector_push ( proc->conections, &conn );
        }

      if ( proc )
        proc->total_conections = vector_size ( proc->conections );
    }

  free ( fds );
  free ( pids );

  procs->proc = copy_ht_to_array ( ht_process, procs->proc );
  procs->total = ht_process->nentries;

  return 1;
}

void
processes_free ( struct processes *processes )
{
  if ( !processes )
    return;

  free ( processes->proc );
  free ( processes );
  hashtable_destroy ( ht_process );
}
