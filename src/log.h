#ifndef LOG_FILE
#define LOG_FILE

#include "process.h"
#include "rate.h"

#define TAXA 14
#define LOG_PROG_NAME_LEN 61

typedef struct log_processes
{
  char name[LOG_PROG_NAME_LEN];
  nstats_t tot_Bps_rx;  // trafego total
  nstats_t tot_Bps_tx;
} log_processes;

FILE *
setup_log_file ( const struct config_op * );

int
log_to_file ( process_t *restrict processes,
              const size_t tot_process,
              log_processes **process_filtred,
              size_t *len_buffer_log,
              FILE *restrict log_file );

#endif  // LOG_FILE
