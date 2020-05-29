
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

#ifndef SHOW_H
#define SHOW_H

#include "process.h" // type process_t, MAX_NAME

// tamanho fixo de caracteres até a coluna program
#define PROGRAM 49

#define COLS_PAD PROGRAM + MAX_NAME

// exibe os processos e suas estatisticas de rede
void
show_process ( const process_t *const processes, const size_t tot_process );

void
ui_tick ();

#endif  // SHOW_H
