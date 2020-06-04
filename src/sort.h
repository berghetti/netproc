#ifndef SORT_H
#define SORT_H
#include <stdlib.h>
#include "process.h"
#include "conection.h"

// parameter mode of function sort
#define S_PID   1
#define PPS_TX  2
#define PPS_RX  3
#define RATE_TX 4
#define RATE_RX 5
#define TOT_TX  6
#define TOT_RX  7

// total de colunas que se pode ordenar
#define COLS_TO_SORT 7

void
sort ( process_t *proc, int tot_process, int mode );

#endif  // SORT_H
