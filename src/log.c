
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
#include <stdbool.h>
#include <string.h>

#include "log.h"
#include "vector.h"
#include "human_readable.h"
#include "m_error.h"

struct log_processes
{
  char *name;
  nstats_t tot_Bps_rx;  // trafego total
  nstats_t tot_Bps_tx;
};

static struct log_processes *log_processes = NULL;
static size_t len_log_processes = 0;
static FILE *file = NULL;

// space justify column
#define TAXA 14

// TAXA + TAXA + PROGRAM + '\n'
#define LEN_HEADER TAXA * 2 + 7 + 1

static void
write_process_to_file ( struct log_processes *processes,
                        const size_t tot_process,
                        FILE *file )
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

      fprintf ( file,
                "%*s%*s%s\n",
                -TAXA,
                tx_tot,
                -TAXA,
                rx_tot,
                processes[i].name );
    }
}

static char *
copy_name ( const char *name, size_t size )
{
  char *p = malloc ( size + 1 );

  if ( p )
    {
      memcpy ( p, name, size );
      p[size] = '\0';
    }

  return p;
}

// percorre todos os processos e busca se um processo ( ou outro com mesmo nome,
// assim podemos ter no arquivo de log, os dados total sobre um programa,
// não apenas sobre uma execução do programa),
// ja esta presente no buffer espelho do arquivo de log
// se estiver present incrementa as estaticias de rede desse processo, caso não
// adiciona ao buffer ( e consequentement sera exibido no arquivo de log)
static bool
update_log_process ( process_t **new_procs )
{
  while ( *new_procs )
    {
      process_t *proc = *new_procs++;

      struct log_processes *log = log_processes;

      size_t j;
      for ( j = 0; j < len_log_processes; j++, log++ )
        {
          if ( strcmp ( proc->name, log->name ) )
            continue;

          log->tot_Bps_rx += proc->net_stat.bytes_last_sec_rx;
          log->tot_Bps_tx += proc->net_stat.bytes_last_sec_tx;

          // only one process with same name exist in this buffer
          break;
        }

      // not found, add new process in buffer
      if ( j == len_log_processes )
        {
          log->name = copy_name ( proc->name, strlen ( proc->name ) );
          if ( !log->name )
            return false;

          log->tot_Bps_rx = proc->net_stat.tot_Bps_rx;
          log->tot_Bps_tx = proc->net_stat.tot_Bps_tx;
          len_log_processes++;

          if ( !vector_push ( log_processes, log ) )
            return false;
        }
    }

  return true;
}

int
log_init ( const char *path_log )
{
  file = fopen ( path_log, "w+" );

  if ( !file )
    {
      ERROR_DEBUG ( "Error open/create file '%s': %s",
                    path_log,
                    strerror ( errno ) );
      return 0;
    }

  log_processes = vector_new ( sizeof ( struct log_processes ) );

  if ( !log_processes )
    {
      fclose ( file );
      ERROR_DEBUG ( "%s", "Error create vector log processes" );

      return 0;
    }

  fprintf ( file,
            "%*s%*s%s\n",
            -TAXA,
            "TOTAL TX",
            -TAXA,
            "TOTAL RX",
            "PROGRAM" );

  return 1;
}

int
log_file ( process_t **processes )
{
  if ( !update_log_process ( processes ) )
    return 0;

  // set file one line below header
  fseek ( file, LEN_HEADER, SEEK_SET );
  write_process_to_file ( log_processes, len_log_processes, file );

  return 1;
}

void
log_free ( void )
{
  if ( file )
    fclose ( file );

  if ( log_processes )
    {
      for ( size_t i = 0; i < len_log_processes; i++ )
        free ( log_processes[i].name );

      vector_free ( log_processes );
    }
}
