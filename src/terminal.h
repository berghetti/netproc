
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

#ifndef TERMINAL_H
#define TERMINAL_H

#include <ncurses.h>

#include "config.h"

extern WINDOW *pad;

// carrega os parametros do terminal
bool
setup_terminal ( void );

// start ncurses
bool
setup_ui ( struct config_op *co );

void
resize_pad(const int l, const int c);

// volta configurações original do terminal
void
restore_terminal ( void );

// limpa a tela, podendo tambem limpar o buffer do scroll se disponivel
// void
// clear_cmd ( );

#endif  // TERMINAL_H
