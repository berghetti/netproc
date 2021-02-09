#ifndef LOG_FILE
#define LOG_FILE

FILE *
create_log_file ( const process_t *, const size_t, const struct config_op * );

int
log_to_file ( const process_t *restrict processes,
              size_t tot_process,
              FILE *restrict log_file );

#endif  // LOG_FILE
