
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

#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>

#include "config.h"
#include "connection.h"
#include "directory.h"
#include "rate.h"

typedef struct process
{
  struct net_stat net_stat;   // network statistics
  connection_t **conections;  // connections of process
  char *name;                 // process name
  pid_t pid;                  // process pid
  uint32_t total_conections;  // total process connections

  bool active;  // check if processes is active in update of processes
} process_t;

struct processes
{
  process_t **proc;
  size_t total;
};

struct processes *
processes_init ( void );

int
processes_update ( struct processes *procs, struct config_op *co );

void
processes_free ( struct processes *procs );

#endif  // PROCESS_H
