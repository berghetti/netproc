
#include <stdio.h>   // FILE
#include <stdlib.h>  // free
#include <string.h>  // getline, strstr
#include <unistd.h>  // raedlink
#include <sys/sendfile.h>
#include <stdbool.h>

#include "process.h"
#include "m_error.h"

#define PID -10  // negativo alinhado a esquerda
#define TAXA -13
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
      if (!sscanf ( processes[i].name, "%59s", prog_name ))
        return;

      fprintf ( log_file,
                "%*d %*s %*s %*s\n",
                PID,
                processes[i].pid,
                TAXA,
                processes[i].net_stat.tx_tot,
                TAXA,
                processes[i].net_stat.rx_tot,
                PROGRAM,
                prog_name );
    }
}

// static char *
// get_name_file ( FILE *file )
// {
//   char fd_path[255];
//   char *filename = malloc ( 255 );
//   ssize_t n;
//   int fd = fileno ( file );
//
//   sprintf ( fd_path, "/proc/self/fd/%d", fd );
//   n = readlink ( fd_path, filename, 255 );
//   filename[n] = '\0';
//
//   return filename;
// }

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
            "%*s %*s %*s %*s\n",
            PID,
            "PID",
            TAXA,
            "TOTAL TX",
            TAXA,
            "TOTAL RX",
            PROGRAM,
            "PROGRAM" );

  fprintf ( stderr, "arquivo criado\n" );
  return fd;
}

int
log_to_file ( const process_t *restrict processes,
              size_t tot_process,
              FILE *restrict log_file )
{
  // bool locate;
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;
  // long prev_line = -1;
  pid_t temp_pid = -1;

  // fprintf(stderr, "sizeof *process: %ld\n", sizeof(*processes));

  process_t *process_temp = calloc ( tot_process, sizeof ( *processes ) );
  size_t count = 0;

  rewind ( log_file );

  // fprintf ( stderr, "write file\n" );
  // fprintf ( file_tmp, "%s", line );

  // fprintf ( stderr, "debug pid: %d\n", temp_pid );
  for ( size_t j = 0; j < tot_process; j++ )
    {
      fprintf(stderr, "checking process: \npid: %d\nprocess: %s\n", processes[j].pid, processes[j].name);
      if ( !processes[j].net_stat.tot_Bps_rx &&
           !processes[j].net_stat.tot_Bps_tx )
        continue;

      // memcpy(&process_temp[count], &processes[j], sizeof ( process_t ));
      process_temp[count++] = processes[j];
      fprintf(stderr, "copied process: %s\n", process_temp[count].name);
    }

  // fprintf ( stderr, "count1: %d\n", count);

  // skip header
  getline ( &line, &len, log_file );

  while ( ( nread = getline ( &line, &len, log_file ) ) != -1 )
    {
      sscanf ( line, "%d", &temp_pid );

      fprintf(stderr, "line: %s\n",line);
      // fprintf ( stderr, "debug pid: %d\n", temp_pid );
      for ( size_t i = 0; i < count; i++ )
        {
          fprintf ( stderr, "debug t_pid: %d\n", temp_pid );
          fprintf ( stderr, "debug p_pid: %d\n", process_temp[i].pid );
          if ( temp_pid == process_temp[i].pid )
            {
              // fprintf ( stderr, "math line: %s\n", line );

              // back to begin line (all lines has lenght fixed)
              fseek ( log_file, -( nread ), SEEK_CUR );

              write_process_to_file ( &process_temp[i], 1, log_file );

              // remove already inserted process
              if ( i + 1 < count )
                process_temp[i] = process_temp[count - 1];

              --count;

              break;
            }
        }
    }

  free ( line );

  // fprintf ( stderr, "count2: %d\n", count);
  if ( count )
    {
      // for (size_t i = 0; i < count; i++) {
      //     fprintf(stderr, "escrevendo: \npid: %d\nprocess: %s\n", process_temp[i].pid, process_temp[i].name);
      // }
        write_process_to_file ( process_temp, count, log_file );
    }


  free(process_temp);

  return 1;
}
