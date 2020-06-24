
/*
 *  Copyright (C) 2020 Mayco S. Berghetti
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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
#include "process.h"
#include "conection.h"

// parameter mode of function sort
// #define S_PID   1
// #define PPS_TX  2
// #define PPS_RX  3
// #define RATE_TX 4
// #define RATE_RX 5
// #define TOT_TX  6
// #define TOT_RX  7

enum cols_sort
{
  S_PID = 0,
  PPS_TX,
  PPS_RX,
  RATE_TX,
  RATE_RX,
  TOT_TX,
  TOT_RX,
  COLS_TO_SORT  // total de elementos no enum
};

void
sort ( process_t *proc, int tot_process, int mode );

#endif  // SORT_H
