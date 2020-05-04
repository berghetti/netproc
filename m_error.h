#ifndef ERROR_H
#define ERROR_H

#ifdef USE_ANSI
#define ERROR "\x1b[31m[ERROR]\x1b[0m"
#define FATAL "\x1b[31;1m[FATAL]\x1b[0m"
#define INFO "\x1b[33;1m[INFO]\x1b[0m "
#define DEBUG "\x1b[34m[DEBUG]\x1b[0m"
#else
#define ERROR "[ERROR]"
#define FATAL "[FATAL]"
#define INFO "[INFO] "
#define DEBUG "[DEBUG]"
#endif

// exibe mensagem na saida de erro padrão
void
error ( const char *msg, ... );

// exibe mensagem na saida de erro padrão
// e sai com EXIT_FAILURE
void
fatal_error ( const char *msg, ... );

#endif  // ERROR_H
