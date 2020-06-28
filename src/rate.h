
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

#ifndef RATE_H
#define RATE_H

#include <stdlib.h>  // type size_t
#include <stdint.h>  // uint*_t

#include "config.h"

// espaço amostral para calcular a média
// de estatisticas de rede.
// 5 é um bom valor...
#define LEN_BUF_CIRC_RATE 5

// Considerando que a cada 1024 bits ou bytes (bits por segundo ou bytes por
// segundo), caso escolhido o padrão IEC com base 2, ou 1000 bits/bytes caso
// escolhido o padrão SI, com base 10, o valor sera dividido por 1000 ou 1024
// para que possa ser apresentado de forma "legivel por humanos", assim sempre
// teremos algo como: 1023 B/s, 1023.99 KB/s, 1023.99 Mib/s, 1023.99 Gib/s, ou
// 8388608 TiB/s :o então no pior caso teremos umas string com tamanhao de 14
// bytes ja incluido null byte.
#define LEN_STR_RATE 14

typedef uint64_t nstats_t;

struct net_stat
{
  nstats_t pps_rx[LEN_BUF_CIRC_RATE];  // pacotes por segundo, amostras
  nstats_t pps_tx[LEN_BUF_CIRC_RATE];
  nstats_t Bps_rx[LEN_BUF_CIRC_RATE];  // bytes/bits por segundos, amostras
  nstats_t Bps_tx[LEN_BUF_CIRC_RATE];
  char rx_rate[LEN_STR_RATE];  // taxa de download final
  char tx_rate[LEN_STR_RATE];  // taxa de upload final
  char rx_tot[LEN_STR_RATE];   // total de bytes recebidos
  char tx_tot[LEN_STR_RATE];   //                enviados
  nstats_t avg_Bps_rx;         // média de bytes/bits por segundos
  nstats_t avg_Bps_tx;
  nstats_t avg_pps_rx;  // média de pacotes por segundos
  nstats_t avg_pps_tx;
  nstats_t tot_Bps_rx;  // trafego total
  nstats_t tot_Bps_tx;
};

typedef struct process process_t;

void
calc_avg_rate ( process_t *restrict proc,
                const size_t tot_proc,
                const struct config_op *restrict co );

#endif  // RATE_H
