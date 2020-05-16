
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

#ifndef STATISTICS_PROC_H
#define STATISTICS_PROC_H

#include "process.h"
#include <stdbool.h>

// encontra o processo ao qual o fluxo de dados pertence
// e adiciona/incrementa estatisticas de pacotes por segundo
// e total de bytes
bool
add_statistics_in_processes ( process_t *restrict processes,
                              const size_t tot_proc,
                              const struct packet *restrict pkt );

#endif  // STATISTICS_PROC_H
