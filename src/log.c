
#include <stdio.h>   // FILE
#include <stdlib.h>  // free
#include <string.h>  // getline, strstr
#include <unistd.h>  // raedlink
#include <sys/sendfile.h>
#include <stdbool.h>

#include "process.h"
#include "rate.h"
#include "human_readable.h"
#include "m_error.h"

#define PID -10  // negativo alinhado a esquerda
#define TAXA -14
#define PROGRAM -61

static void
write_process_to_file ( const process_t *restrict processes,
                        const size_t tot_process,
                        FILE *restrict log_file )
{
  for ( size_t i = 0; i < tot_process; i++ )
    {
      // if ( !processes[i].net_stat.tot_Bps_rx &&
      //      !processes[i].net_stat.tot_Bps_tx )
      //   continue;

      // if (!processes[i].name)
      //   return;

      char prog_name[60] = {0};
      if ( !sscanf ( processes[i].name, "%59s", prog_name ) )
        return;

      fprintf ( log_file,
                "%*s %*s %*s\n",
                // PID,
                // processes[i].pid,
                TAXA,
                processes[i].net_stat.tx_tot,
                TAXA,
                processes[i].net_stat.rx_tot,
                PROGRAM,
                prog_name );
    }
}

FILE *
create_log_file ( const struct config_op *co )
{
  // file_tmp = fopen("netproc.tmp", "w+");

  FILE *fd = fopen ( co->path_log, "w+" );
  if ( !fd )
    fatal_error ( "Error open/create file '%s': %s",
                  co->path_log,
                  strerror ( errno ) );

  fprintf ( fd,
            "%*s %*s %*s\n",
            // PID,
            // "PID",
            TAXA,
            "TOTAL TX",
            TAXA,
            "TOTAL RX",
            PROGRAM,
            "PROGRAM" );

  fprintf ( stderr, "arquivo criado\n" );
  return fd;
}

static size_t
filter_active_process ( process_t **buffer,
                        const process_t *processes,
                        const size_t tot_process )
{
  size_t count = 0;

  if ( !( *buffer = malloc ( tot_process * sizeof ( process_t ) ) ) )
    fatal_error ( "Error alloc memory: %s", strerror ( errno ) );

  for ( size_t i = 0; i < tot_process; i++ )
    {
      if ( !processes[i].net_stat.tot_Bps_rx &&
           !processes[i].net_stat.tot_Bps_tx )
        continue;

      ( *buffer )[count++] = processes[i];
    }

  return count;
}

int
log_to_file ( const process_t *restrict processes,
              const size_t tot_process,
              FILE *restrict log_file )
{
  // bool locate;
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;
  // long prev_line = -1;
  pid_t temp_pid = -1;

  char prog_name[60] = {0}, prog_name2[60] = {0};

  // nstats_t total_tx, total_rx;

  process_t *process_filtred = NULL;
  size_t tot_process_filtred;
  tot_process_filtred =
          filter_active_process ( &process_filtred, processes, tot_process );

  // join duplicata
  for ( size_t i = 0; i < tot_process_filtred; i++ )
    {
      // total_tx = total_rx = 0;
        sscanf ( process_filtred[i].name, "%59s", prog_name );

      for ( size_t j = 0; j < tot_process_filtred; )
        {
          if ( i == j )
            {
              j++;
              continue;
            }


          sscanf ( process_filtred[j].name, "%59s", prog_name2 );

          fprintf ( stderr, "testando %s e %s\n", prog_name, prog_name2 );

          // strcmp
          if ( !strncmp ( prog_name, prog_name2, strlen ( prog_name ) ) )
            {
              fprintf ( stderr, "%s\n", "match" );

              process_filtred[i].net_stat.tot_Bps_tx +=
                      process_filtred[j].net_stat.tot_Bps_tx;

              human_readable ( process_filtred[i].net_stat.tx_tot,
                               sizeof ( process_filtred[i].net_stat.tx_tot ),
                               process_filtred[i].net_stat.tot_Bps_tx,
                               TOTAL );

              process_filtred[i].net_stat.tot_Bps_rx +=
                      process_filtred[j].net_stat.tot_Bps_rx;

              human_readable ( process_filtred[i].net_stat.rx_tot,
                               sizeof ( process_filtred[i].net_stat.rx_tot ),
                               process_filtred[i].net_stat.tot_Bps_rx,
                               TOTAL );

              // fprintf ( stderr, "rx_rate %s\n",
              // process_filtred[i].net_stat.rx_rate );
              //
              if ( j + 1 < tot_process_filtred )
                {
                  process_filtred[j] = process_filtred[tot_process_filtred-- - 1];
                  continue;
                }


              // tot_process_filtred--;
            }

            j++;
        }
    }

  rewind ( log_file );
  // skip header
  getline ( &line, &len, log_file );

  while ( ( nread = getline ( &line, &len, log_file ) ) != -1 )
    {
      // sscanf ( line, "%*d %*d %59s", prog_name );
      sscanf ( line, "%*28c%59s", prog_name );

      fprintf ( stderr, "line: %s\n", line );
      // fprintf ( stderr, "debug pid: %d\n", temp_pid );
      for ( size_t i = 0; i < tot_process_filtred; i++ )
        {
          // fprintf ( stderr, "debug t_pid: %d\n", temp_pid );
          // fprintf ( stderr, "debug p_pid: %d\n", process_filtred[i].pid );

          sscanf ( process_filtred[i].name, "%59s", prog_name2 );

          fprintf (
                  stderr, "testando in file %s e %s\n", prog_name, prog_name2 );

          if ( !strncmp ( prog_name, prog_name2, strlen ( prog_name ) ) )
            {
              fprintf ( stderr, "%s\n", "match file" );

              // back to begin line (all lines has lenght fixed)
              fseek ( log_file, -( nread ), SEEK_CUR );

              write_process_to_file ( &process_filtred[i], 1, log_file );

              // remove already inserted process
              if ( i + 1 < tot_process_filtred )
                process_filtred[i] = process_filtred[tot_process_filtred - 1];

              tot_process_filtred--;

              break;
            }
        }
    }

  free ( line );

  fprintf ( stderr, "tot_process_filtred2: %lu\n", tot_process_filtred );
  if ( tot_process_filtred )
    {
      // for (size_t i = 0; i < tot_process_filtred; i++) {
      //     fprintf(stderr, "escrevendo: \npid: %d\nprocess: %s\n",
      //     process_filtred[i].pid, process_filtred[i].name);
      //      write_process_to_file ( &process_filtred[i], 1, log_file );
      // }
      write_process_to_file ( process_filtred, tot_process_filtred, log_file );
    }

  free ( process_filtred );

  return 1;
}
