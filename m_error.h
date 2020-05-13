#ifndef ERROR_H
#define ERROR_H

#define ERROR "[ERROR]"
#define FATAL "[FATAL]"
#define DEBUG "[DEBUG]"

// exibe mensagem na saida de erro padrão
void
error ( const char *msg, ... );

// exibe mensagem na saida de erro padrão
// e sai com EXIT_FAILURE
void
fatal_error ( const char *msg, ... );

#endif  // ERROR_H
