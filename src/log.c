
#include <stdio.h>   // FILE
#include <stdlib.h>  // free
#include <string.h>  //
#include <unistd.h>  // raedlink

#include "process.h"
#include "rate.h"
#include "str.h"
#include "human_readable.h"
#include "m_error.h"

#define TAXA 14
#define PROGRAM 61  // PROGRAM -1 is max lenght of name write in file

// qts of process firt len buffer processes to file log
#define FIRST_LEN_LOG 12

typedef struct log_processes
{
  char name[PROGRAM];
  char rx_tot[LEN_STR_RATE];  // total de bytes recebidos
  char tx_tot[LEN_STR_RATE];  //                enviados
  nstats_t tot_Bps_rx;        // trafego total
  nstats_t tot_Bps_tx;
  pid_t pid;
  bool print;
} log_processes;

static size_t last_pos = 0;
static size_t max_process_log = FIRST_LEN_LOG;
static log_processes *process_filtred = NULL;

static void *
alloc_processes_log ( struct log_processes **l, size_t len )
{
  void *temp = NULL;
  // fprintf ( stderr, "alocando %ld bytes\n", sizeof ( struct log_processes ) );
  if ( !( temp = realloc ( *l, len * sizeof ( struct log_processes ) ) ) )
    fatal_error ( "Error (re)alloc memory: %s", strerror ( errno ) );

  *l = temp;
  return l;
}

static void
write_process_to_file ( struct log_processes *restrict processes,
                        const size_t tot_process,
                        FILE *restrict log_file )
{
  for ( size_t i = 0; i < tot_process; i++ )
    {
      // if ( !processes[i].tot_Bps_rx && !processes[i].tot_Bps_tx )
      //   continue;
      if ( !processes[i].print )
        continue;

      human_readable ( processes[i].tx_tot,
                       sizeof ( processes[i].tx_tot ),
                       processes[i].tot_Bps_tx,
                       TOTAL );

      human_readable ( processes[i].rx_tot,
                       sizeof ( processes[i].rx_tot ),
                       processes[i].tot_Bps_rx,
                       TOTAL );
      fprintf ( log_file,
                "%*s %*s %*s\n",
                -TAXA,
                processes[i].tx_tot,
                -TAXA,
                processes[i].rx_tot,
                -PROGRAM,
                processes[i].name );
    }
}

// static void
// start_log_processes ( const process_t *processes, const size_t tot_proc )
// {
//   alloc_processes_log ( &process_filtred, tot_proc );
//   tot_process_log = tot_proc;
//
//   for ( size_t i = 0; i < tot_proc; i++ )
//     {
//       sscanf ( processes[i].name, "%59s", process_filtred[i].name );
//       process_filtred[i].tot_Bps_tx = processes[i].net_stat.tot_Bps_tx;
//       process_filtred[i].tot_Bps_rx = processes[i].net_stat.tot_Bps_rx;
//     }
// }

FILE *
create_log_file ( const process_t *processes,
                  const size_t tot_proc,
                  const struct config_op *co )
{
  FILE *fd = fopen ( co->path_log, "w+" );
  if ( !fd )
    fatal_error ( "Error open/create file '%s': %s",
                  co->path_log,
                  strerror ( errno ) );

  alloc_processes_log ( &process_filtred, max_process_log );

  // start_log_processes ( processes, tot_proc );

  fprintf ( fd,
            "%*s %*s %*s\n",
            -TAXA,
            "TOTAL TX",
            -TAXA,
            "TOTAL RX",
            -PROGRAM,
            "PROGRAM" );

  return fd;
}

// add statistics of network in only one processes of all processes that same
// name
static size_t
join_processes_same_name ( struct log_processes *processes,
                           size_t tot_processes )
{
  size_t len_name;
  size_t i, j;

  for ( i = 0; i < tot_processes; i++ )
    {
      // int k = 0;
      // for ( char c = processes[i].name[k]; c; k++ )
      //   {
      //     fprintf ( stderr, "k = %d c = %c\n", k, processes[i].name[k] );
      //     c = processes[i].name[k];
      //   }

      len_name = strlen ( process_filtred[i].name );
      // fprintf ( stderr, "len_name %ld\n", len_name );

      j = i + 1;
      while ( j < tot_processes )
        {
          // fprintf ( stderr,
          //           "testando %d:%s e %d:%s\n",
          //           processes[i].pid,
          //           processes[i].name,
          //           processes[j].pid,
          //           processes[j].name );

          if ( !strncmp ( process_filtred[i].name, process_filtred[j].name, len_name ) )
            {
              process_filtred[j].print = false;
              // fprintf ( stderr, "%s\n", "match" );

              process_filtred[i].tot_Bps_tx += process_filtred[j].tot_Bps_tx;
              processes[i].tot_Bps_rx += process_filtred[j].tot_Bps_rx;

              // needle to not re-accumulate value
              // this processes are secondary
              process_filtred[j].tot_Bps_tx = 0;
              process_filtred[j].tot_Bps_rx = 0;
            }

          j++;
        }
    }

  return tot_processes;
}

static void
copy_process_to_log ( process_t *restrict process, log_processes *restrict log )
{
  // fprintf ( stderr, "salvando %d %s\n", process->pid, process->name );
  log->print = true;
  log->pid = process->pid;
  sscanf ( process->name, "%59s", log->name );
  // log->name[PROGRAM] = '\0';

  log->tot_Bps_rx = process->net_stat.tot_Bps_rx;
  log->tot_Bps_tx = process->net_stat.tot_Bps_tx;
}

static void
update_log_process ( process_t *new_proc,
                     const size_t len_proc,
                     log_processes *log,
                     const size_t tot_log )
{
  char temp[PROGRAM];
  size_t len_name;
  bool locate;

  for ( size_t i = 0; i < len_proc; i++ )
    {
      sscanf ( new_proc[i].name, "%59s", temp );

      len_name = strlen ( temp );
      locate = false;

      for ( size_t j = 0; j < tot_log; j++ )
        {
          // fprintf ( stderr,
          //           "testando %d:%s e %d:%s\n",
          //           new_proc[i].pid,
          //           temp,
          //           log[j].pid,
          //           log[j].name );

          if ( new_proc[i].pid == process_filtred[j].pid &&
               !strncmp ( temp, process_filtred[j].name, len_name ) )
            {
              // fprintf ( stderr, "match\n" );
              locate = true;

              // fprintf(stderr, "incrementando %ld\n",
              // new_proc[i].net_stat.bytes_last_sec_rx);
              process_filtred[j].tot_Bps_rx += new_proc[i].net_stat.bytes_last_sec_rx;
              process_filtred[j].tot_Bps_tx += new_proc[i].net_stat.bytes_last_sec_tx;
              break;
            }
        }

      if ( !locate )
        {
          if ( last_pos == max_process_log )
            {
              max_process_log <<= 1;  // double
              alloc_processes_log ( &process_filtred, max_process_log );
            }

          copy_process_to_log ( &new_proc[i], &process_filtred[last_pos++] );
        }
    }
}

int
log_to_file ( process_t *restrict processes,
              const size_t tot_process,
              FILE *restrict log_file )
{

  update_log_process ( processes, tot_process, process_filtred, last_pos );

  join_processes_same_name ( process_filtred, last_pos );

  rewind ( log_file );
  write_process_to_file ( process_filtred, last_pos, log_file );


  return 1;
}
