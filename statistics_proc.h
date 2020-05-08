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
