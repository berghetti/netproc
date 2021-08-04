
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

#ifndef SHOW_H
#define SHOW_H

#include "processes.h"  // type process_t, MAX_NAME
#include "config.h"

// return of tui_handle_input
#define P_EXIT 1
#define P_CONTINE 0

int
tui_init ( const struct config_op *co );

void
tui_show ( const struct processes *processes, const struct config_op *co );

// handle input of user while program is running
int
tui_handle_input ( const struct config_op *co );

void
tui_free ( void );

#endif  // SHOW_H
