
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
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

#ifndef SHOW_H
#define SHOW_H

#include "process.h"  // type process_t, MAX_NAME
#include "config.h"

// tamanho fixo de caracteres até a coluna program
#define PROGRAM 77

#define COLS_PAD PROGRAM + MAX_NAME
#define LINES_PAD 1000

// exibe os processos e suas estatisticas de rede
void
show_process ( const process_t *restrict processes,
               const size_t tot_process,
               const struct config_op *restrict co );

// inicia a primeira vez a interface do usuario
void
start_ui ( const struct config_op *co );

// return of running_input
#define P_EXIT 1
#define P_CONTINE 0

// handle input of user while program is running
int
running_input ( const struct config_op *co );

#endif  // SHOW_H
