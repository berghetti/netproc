
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

#ifndef STATISTICS_PROC_H
#define STATISTICS_PROC_H

#include <stdbool.h>

#include "config.h"
#include "processes.h"

// encontra o processo ao qual o fluxo de dados pertence
// e adiciona/incrementa estatisticas de pacotes por segundo
// e total de bytes
bool
add_statistics_in_processes ( struct processes *processes,
                              const struct packet *pkt,
                              const struct config_op *co );

#endif  // STATISTICS_PROC_H
