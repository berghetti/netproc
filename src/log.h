#ifndef LOG_FILE
#define LOG_FILE

FILE *
create_log_file ( const struct config_op *co );

int
log_to_file ( const process_t *restrict processes,
              size_t tot_process,
              FILE *restrict log_file );

#endif  // LOG_FILE
