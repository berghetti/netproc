
#include <stdio.h>   // FILE
#include <stdlib.h>  // free
#include <string.h>  //
#include <unistd.h>  // raedlink

#include "log.h"
#include "str.h"
#include "human_readable.h"
#include "m_error.h"



// qts of process firt len buffer processes to file log
#define FIRST_LEN_LOG 12

// typedef struct log_processes
// {
//   char name[PROGRAM];
//   nstats_t tot_Bps_rx;  // trafego total
//   nstats_t tot_Bps_tx;
// } log_processes;


static void
alloc_processes_log ( struct log_processes **buff, const size_t len )
{
  if ( !( *buff = realloc ( *buff, len * sizeof ( **buff ) ) ) )
    fatal_error ( "Error (re)alloc memory: %s", strerror ( errno ) );
}

static void
write_process_to_file ( struct log_processes *restrict processes,
                        const size_t tot_process,
                        FILE *restrict log_file )
{
  char rx_tot[LEN_STR_RATE - 2];
  char tx_tot[LEN_STR_RATE - 2];

  for ( size_t i = 0; i < tot_process; i++ )
    {
      // not log process whitout traffic
      if ( !processes[i].tot_Bps_rx && !processes[i].tot_Bps_tx )
        continue;

      human_readable (
              tx_tot, LEN_STR_RATE - 2, processes[i].tot_Bps_tx, TOTAL );

      human_readable (
              rx_tot, LEN_STR_RATE - 2, processes[i].tot_Bps_rx, TOTAL );

      fprintf ( log_file,
                "%*s%*s%*s\n",
                -TAXA,
                tx_tot,
                -TAXA,
                rx_tot,
                -LOG_PROG_NAME_LEN,
                processes[i].name );
    }
}

FILE *
setup_log_file ( const struct config_op *co )
{
  FILE *fd = fopen ( co->path_log, "w+" );
  if ( !fd )
    fatal_error ( "Error open/create file '%s': %s",
                  co->path_log,
                  strerror ( errno ) );

  // alloc_processes_log ( &process_filtred, max_process_log );

  fprintf ( fd,
            "%*s%*s%*s\n",
            -TAXA,
            "TOTAL TX",
            -TAXA,
            "TOTAL RX",
            -LOG_PROG_NAME_LEN,
            "PROGRAM" );

  return fd;
}

// percorre todos os processos e busca se um processo ( ou outro com mesmo nome,
// assim podemos ter no arquivo de log, os dados total sobre um programa,
// não apenas sobre uma executação do programa),
// ja esta presente no buffer espelho do arquivo de log
// se estiver present incrementa as estaticias de rede desse processo, caso não
// adiciona ao buffer ( e consequentement sera exibido no arquivo de log)
static void
update_log_process ( const process_t *new_proc,
                     const size_t len_proc,
                     log_processes **buff_log,
                     size_t *len_buff )
{
  static size_t max_len_buff_log = 0;
  char buffer_name[LOG_PROG_NAME_LEN];
  size_t len_name;
  bool locate;

  // fprintf(stderr, "tot buff %ld\n", *len_buff);

  for ( size_t i = 0; i < len_proc; i++ )
    {
      sscanf ( new_proc[i].name, "%60s",  buffer_name );

      len_name = strlen ( buffer_name );
      locate = false;

      for ( size_t j = 0; j < *len_buff; j++ )
        {
          if ( !strncmp ( buffer_name, ( *buff_log )[j].name, len_name ) )
            {
              locate = true;

              ( *buff_log )[j].tot_Bps_rx +=
                      new_proc[i].net_stat.bytes_last_sec_rx;
              ( *buff_log )[j].tot_Bps_tx +=
                      new_proc[i].net_stat.bytes_last_sec_tx;

              // only one process with same name exist in this buffer
              break;
            }
        }

      // add new process in buffer
      if ( !locate )
        {
          if ( *len_buff == max_len_buff_log || !max_len_buff_log)
            {
              if (!max_len_buff_log)
                max_len_buff_log = FIRST_LEN_LOG;

              max_len_buff_log <<= 1;  // double
              alloc_processes_log ( buff_log, max_len_buff_log );
            }

          strncpy ( ( *buff_log )[*len_buff].name,
                    buffer_name,
                    sizeof ( ( *buff_log )[*len_buff].name ) );

          ( *buff_log )[*len_buff].tot_Bps_rx = new_proc[i].net_stat.tot_Bps_rx;
          ( *buff_log )[*len_buff].tot_Bps_tx = new_proc[i].net_stat.tot_Bps_tx;
          (*len_buff)++;
        }
    }
}

void
free_buff_file()
{

}

int
log_to_file ( process_t *restrict processes,
              const size_t tot_process,
              log_processes **process_filtred,
              size_t *len_buffer_log,
              FILE *restrict log_file )
{
  // static log_processes *process_filtred = NULL;
  // static size_t len_buffer_log = 0;

  update_log_process (
          processes, tot_process, process_filtred, len_buffer_log );

  // set file one line below header
  fseek ( log_file, TAXA * 2 + LOG_PROG_NAME_LEN + 1, SEEK_SET );
  write_process_to_file ( *process_filtred, *len_buffer_log, log_file );

  return 1;
}
