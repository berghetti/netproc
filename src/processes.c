
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
#include <netinet/tcp.h>  // TCP_ESTABLISHED, TCP_TIME_WAIT...

#include "hashtable.h"
#include "full_read.h"
#include "processes.h"  // process_t
#include "config.h"
#include "m_error.h"  // ERROR_DEBUG
#include "macro_util.h"

// /proc/%d/cmdline
#define MAX_CMDLINE 25

//   6  +  7  + 4 + 7
// /proc/<pid>/fd/<id-fd>
#define MAX_PATH_FD 24

// strlen ("socket:[99999999]") + 3 safe
#define MAX_NAME_SOCKET 9 + 8 + 3

static hashtable_t *ht_process;

static conection_t *
add_conection_to_process ( process_t *proc, conection_t *con )
{
  proc->total_conections++;
  conection_t *cons = realloc ( proc->conection,
                                proc->total_conections * sizeof ( *cons ) );

  if ( cons )
    {
      proc->conection = cons;
      proc->conection[proc->total_conections - 1] = *con;
    }

  return cons;
}

// armazena o nome do processo no buffer e retorna
// o tamanho do nome do processo,
// função cuida da alocação de memoria para o nome do processo
static ssize_t
get_name_process ( char **buffer, const pid_t pid )
{
  char path_cmdline[MAX_CMDLINE];
  snprintf ( path_cmdline, MAX_CMDLINE, "/proc/%d/cmdline", pid );

  int fd = open ( path_cmdline, O_RDONLY );
  if ( fd == -1 )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return -1;
    }

  ssize_t total_read = full_read ( fd, buffer );
  close ( fd );

  if ( total_read == -1 )
    return -1;

  char *p = *buffer;

  // run (total_read -1) times
  size_t i = total_read;
  while ( --i )
    {
      if ( *p == '\0' || *p == '\n' )
        *p = ' ';

      p++;
    }

  // warranty, already null terminated
  *p = 0;

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
      proc->total_conections = 0;
      proc->active = 1;
      proc->conection = NULL;
      get_name_process ( &proc->name, pid );

      memset ( &proc->net_stat, 0, sizeof ( struct net_stat ) );
    }

  return proc;
}

static void
free_process ( void *arg )
{
  process_t *process = arg;
  free ( process->name );
  free ( process->conection );
  free ( process );
}

static int
free_dead_process ( hashtable_t *ht, void *value, UNUSED ( void *user_data ) )
{
  process_t *proc = ( process_t * ) value;

  if ( !proc->active )
    free_process ( hashtable_remove ( ht, TO_PTR ( proc->pid ) ) );

  return 0;
}

struct my_array
{
  process_t **data;
  size_t pos;
};

static int
to_array ( UNUSED ( hashtable_t *ht ), void *value, void *user_data )
{
  struct my_array *ar = user_data;
  process_t *proc = value;
  proc->active = 0;  // reset status process

  ar->data[ar->pos] = proc;
  ar->pos++;

  return 0;
}

static process_t **
copy_ht_to_array ( hashtable_t *ht, process_t **proc )
{
  process_t **pp = realloc ( proc, ( ht->nentries + 1 ) * sizeof ( *pp ) );

  if ( pp )
    {
      struct my_array my_array = { .data = pp, .pos = 0 };

      hashtable_foreach ( ht, to_array, &my_array );

      pp[ht->nentries] = NULL;  // last pointer
    }

  return pp;
}

static int
cb_compare ( const void *key1, const void *key2 )
{
  return ( key1 == key2 );
}

// https://github.com/shemminger/iproute2/blob/main/misc/ss.c
static hash_t
cb_hash_func ( const void *key )
{
  size_t k = ( size_t ) FROM_PTR ( key );

  return ( k >> 24 ) ^ ( k >> 16 ) ^ ( k >> 8 ) ^ k;
}

struct processes *
processes_init ( void )
{
  struct processes *procs = calloc ( 1, sizeof *procs );
  if ( !procs )
    return NULL;

  ht_process = hashtable_new ( cb_hash_func, cb_compare, free_process );
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
  uint32_t *pids = NULL;
  int total_process = get_numeric_directory ( &pids, "/proc/" );

  if ( total_process == -1 )
    {
      ERROR_DEBUG ( "%s", "backtrace" );
      return 0;
    }

  conection_t *conections;
  int total_conections = get_conections ( &conections, co->proto );

  if ( -1 == total_conections )
    {
      ERROR_DEBUG ( "%s", "backtrace" );
      free ( pids );
      return 0;
    }

  uint32_t *fds = NULL;
  for ( int index_pid = 0; index_pid < total_process; index_pid++ )
    {
      char path_fd[MAX_PATH_FD];
      snprintf ( path_fd, MAX_PATH_FD, "/proc/%d/fd/", pids[index_pid] );

      int total_fd_process = get_numeric_directory ( &fds, path_fd );

      if ( total_fd_process == -1 )
        continue;

      process_t *proc =
              hashtable_get ( ht_process, TO_PTR ( pids[index_pid] ) );

      if ( proc && ( proc->net_stat.tot_Bps_rx || proc->net_stat.tot_Bps_tx ) )
        {
          proc->active = 1;
          proc->total_conections = 0;
        }

      for ( int index_fd = 0; index_fd < total_fd_process; index_fd++ )
        {
          snprintf ( path_fd,
                     MAX_PATH_FD,
                     "/proc/%d/fd/%d",
                     pids[index_pid],
                     fds[index_fd] );

          char data_fd[MAX_NAME_SOCKET];
          ssize_t len_link = readlink ( path_fd, data_fd, MAX_NAME_SOCKET );

          if ( len_link == -1 )
            continue;

          data_fd[len_link] = '\0';

          if ( data_fd[0] != 's' &&
               strncmp ( data_fd + 1, "ocket:[", strlen ( "ocket:[" ) ) )
            continue;

          for ( int c = 0; c < total_conections; c++ )
            {
              if ( conections[c].state == TCP_TIME_WAIT ||
                   conections[c].state == TCP_LISTEN )
                continue;

              char socket[MAX_NAME_SOCKET];
              snprintf ( socket,
                         MAX_NAME_SOCKET,
                         "socket:[%d]",
                         conections[c].inode );

              if ( strncmp ( socket, data_fd, len_link ) )
                continue;

              if ( !proc )
                {
                  proc = create_new_process ( pids[index_pid] );
                  if ( !proc )
                    break;  // no return error, check others processes

                  hashtable_set (
                          ht_process, TO_PTR ( pids[index_pid] ), proc );
                }

              add_conection_to_process ( proc, &conections[c] );
            }
        }
    }

  free ( fds );
  free ( pids );
  free ( conections );

  hashtable_foreach ( ht_process, free_dead_process, NULL );

  procs->total = ht_process->nentries;

  procs->proc = copy_ht_to_array ( ht_process, procs->proc );

  return 1;
}

void
processes_free ( struct processes *processes )
{
  if ( processes )
    {
      free ( processes->proc );
      free ( processes );
    }

  if ( ht_process )
    hashtable_destroy ( ht_process );
}
