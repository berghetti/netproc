
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

#ifndef LOG_FILE
#define LOG_FILE

#include <stdbool.h>

#include "process.h"
#include "rate.h"

typedef struct log_processes
{
  char *name;
  nstats_t tot_Bps_rx;  // trafego total
  nstats_t tot_Bps_tx;
} log_processes;

FILE *
setup_log_file ( const struct config_op * );

bool
log_to_file ( process_t *restrict processes,
              const size_t tot_process,
              log_processes **process_filtred,
              size_t *len_buffer_log,
              FILE *restrict log_file );

void
free_log ( FILE *file, log_processes *log, size_t len );

#endif  // LOG_FILE
