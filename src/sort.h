
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

#ifndef SORT_H
#define SORT_H

#include <stdlib.h>
#include "processes.h"
#include "connection.h"

enum cols_sort
{
  S_PID = 0,
  PPS_TX,
  PPS_RX,
  RATE_TX,
  RATE_RX,
  TOT_TX,
  TOT_RX,
  COLS_TO_SORT  // total elements in enum
};

void
sort ( process_t **proc,
       size_t tot_process,
       int mode,
       const struct config_op *co );

#endif  // SORT_H
