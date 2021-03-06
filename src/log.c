
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

#include <stdio.h>   // FILE
#include <stdlib.h>  // free
#include <string.h>

#include "log.h"
#include "str.h"
#include "human_readable.h"
#include "m_error.h"

// qts of process firt len buffer processes to file log
#define FIRST_LEN_LOG 12

// space justify column
#define TAXA 14

// TAXA + TAXA + PROGRAM + '\n'
#define LEN_HEADER TAXA * 2 + 7 + 1

static void *
alloc_processes_log ( struct log_processes **buff, const size_t len )
{
  if ( !( *buff = realloc ( *buff, len * sizeof ( **buff ) ) ) )
    {
      ERROR_DEBUG ( "\"%s\"", strerror ( errno ) );
      return NULL;
    }

  return *buff;
}

static void
write_process_to_file ( struct log_processes *restrict processes,
                        const size_t tot_process,
                        FILE *restrict log_file )
{
  char rx_tot[LEN_STR_TOTAL];
  char tx_tot[LEN_STR_TOTAL];

  for ( size_t i = 0; i < tot_process; i++ )
    {
      // not log process whitout traffic
      if ( !processes[i].tot_Bps_rx && !processes[i].tot_Bps_tx )
        continue;

      human_readable ( tx_tot, sizeof tx_tot, processes[i].tot_Bps_tx, TOTAL );

      human_readable ( rx_tot, sizeof rx_tot, processes[i].tot_Bps_rx, TOTAL );

      fprintf ( log_file,
                "%*s%*s%s\n",
                -TAXA,
                tx_tot,
                -TAXA,
                rx_tot,
                processes[i].name );
    }
}

FILE *
setup_log_file ( const struct config_op *co )
{
  FILE *fd = fopen ( co->path_log, "w+" );
  if ( !fd )
    {
      ERROR_DEBUG ( "Error open/create file '%s': %s",
                    co->path_log,
                    strerror ( errno ) );
      return NULL;
    }

  fprintf ( fd, "%*s%*s%s\n", -TAXA, "TOTAL TX", -TAXA, "TOTAL RX", "PROGRAM" );

  return fd;
}

// percorre todos os processos e busca se um processo ( ou outro com mesmo nome,
// assim podemos ter no arquivo de log, os dados total sobre um programa,
// não apenas sobre uma executação do programa),
// ja esta presente no buffer espelho do arquivo de log
// se estiver present incrementa as estaticias de rede desse processo, caso não
// adiciona ao buffer ( e consequentement sera exibido no arquivo de log)
static bool
update_log_process ( const process_t *new_proc,
                     const size_t len_proc,
                     log_processes **buff_log,
                     size_t *len_buff )
{
  static size_t max_len_buff_log = 0;
  size_t len_name;
  bool locate;

  for ( size_t i = 0; i < len_proc; i++ )
    {
      len_name = strlen ( new_proc[i].name );
      locate = false;

      for ( size_t j = 0; j < *len_buff; j++ )
        {
          if ( !strncmp ( new_proc[i].name, ( *buff_log )[j].name, len_name ) )
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
          if ( *len_buff == max_len_buff_log || !max_len_buff_log )
            {
              if ( !max_len_buff_log )
                max_len_buff_log = FIRST_LEN_LOG;

              max_len_buff_log <<= 1;  // double
              if ( !alloc_processes_log ( buff_log, max_len_buff_log ) )
                return false;
            }

          ( *buff_log )[*len_buff].name =
                  strndup ( new_proc[i].name, len_name );

          ( *buff_log )[*len_buff].tot_Bps_rx = new_proc[i].net_stat.tot_Bps_rx;
          ( *buff_log )[*len_buff].tot_Bps_tx = new_proc[i].net_stat.tot_Bps_tx;
          ( *len_buff )++;
        }
    }

  return true;
}

void
free_log ( FILE *file, log_processes *log, size_t len )
{
  fclose ( file );
  log_processes *temp = log;

  if ( !log )
    return;

  while ( len-- > 0 )
    free ( temp++->name );

  free ( log );
}

bool
log_to_file ( process_t *restrict processes,
              const size_t tot_process,
              log_processes **process_filtred,
              size_t *len_buffer_log,
              FILE *restrict log_file )
{
  if ( !update_log_process (
               processes, tot_process, process_filtred, len_buffer_log ) )
    return false;

  // set file one line below header
  fseek ( log_file, LEN_HEADER, SEEK_SET );
  write_process_to_file ( *process_filtred, *len_buffer_log, log_file );

  return true;
  ;
}
