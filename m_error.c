
#define _GNU_SOURCE // for asprintf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include "m_error.h"

static void print_error(const char *msg, va_list args);

void error(const char *msg, ...)
{
  va_list args;

  fprintf(stderr, ERROR"\n");
  va_start(args, msg);

  print_error(msg, args);

  va_end(args);
}

void fatal_error(const char *msg, ...)
{
  va_list args;

  fprintf(stderr, FATAL"\n");
  va_start(args, msg);

  print_error(msg, args);

  va_end(args);
  exit(EXIT_FAILURE);
}

// inclui '\n' no fim da mensagem e imprime
// na saida de erro padrão
static void
print_error(const char *msg, va_list args)
{
  char *msg_formated;

  errno = 0;
  if (asprintf(&msg_formated, "%s\n", msg) == -1) // inclui '\n' na mensagem
    {
      if (errno) // asprintf set errno in case fault?
        perror("asprintf");
      else
        fprintf(stderr, "%s\n", "asprintf: Unknown error");

      exit(EXIT_FAILURE);
    }

  // exibe a mensagem
  vfprintf(stderr, msg_formated, args);
  free(msg_formated);
}
