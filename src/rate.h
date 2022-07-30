
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

#ifndef RATE_H
#define RATE_H

#include <stdlib.h>  // type size_t
#include <stdint.h>  // uint*_t

#include "config.h"

/* amostral space, from last five seconds */
#define SAMPLE_SPACE_SIZE 5

/*
 Considerando que a cada 1024 bits ou bytes (bits por segundo ou bytes por
 segundo), caso escolhido o padrão IEC com base 2, ou 1000 bits/bytes caso
 escolhido o padrão SI, com base 10, o valor sera dividido por 1000 ou 1024
 para que possa ser apresentado de forma "legivel por humanos", assim sempre
 teremos algo como: 1023 B/s, 1023.99 KB/s, 1023.99 Mib/s, 1023.99 Gib/s, ou
 8388608 TiB/s (isso é um programa de rede, voltado para hosts e não
 roteadores dificilmente teremos trafego maior que isso...), então no pior
 caso teremos umas string com tamanhao de 14 bytes ja incluido null byte. */
#define LEN_STR_RATE 14

// not ended with '/s'
#define LEN_STR_TOTAL LEN_STR_RATE - 2

typedef uint64_t nstats_t;

struct net_stat
{
  // samples pps and bytes per second rx/tx
  nstats_t pps_rx[SAMPLE_SPACE_SIZE];
  nstats_t pps_tx[SAMPLE_SPACE_SIZE];
  nstats_t Bps_rx[SAMPLE_SPACE_SIZE];
  nstats_t Bps_tx[SAMPLE_SPACE_SIZE];

  // averege bytes/second and packets/second rx/tx
  nstats_t avg_Bps_rx;
  nstats_t avg_Bps_tx;
  nstats_t avg_pps_rx;
  nstats_t avg_pps_tx;

  // total bytes traffic rx/tx
  nstats_t tot_Bps_rx;
  nstats_t tot_Bps_tx;

  // used by function log.c/log_file()
  nstats_t bytes_last_sec_rx;  // total bytes in last second
  nstats_t bytes_last_sec_tx;
};

struct processes;

void
rate_add_tx ( struct net_stat *ns, size_t lenght );

void
rate_add_rx ( struct net_stat *ns, size_t lenght );

void
rate_calc ( struct processes *processes, const struct config_op *co );

void
rate_update ( struct processes *processes, const struct config_op *co );

#endif  // RATE_H
