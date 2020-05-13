
#define _GNU_SOURCE  // for asprintf
#include <errno.h>   // variable errno
#include <stdarg.h>  // va_*
#include <stdio.h>
#include <stdlib.h>
#include <term.h>   // tputs
#include <unistd.h>

#include "m_error.h"

// based in source code of program t50
// https://gitlab.com/fredericopissarra/t50/-/blob/master/src/errors.c

#define RED 1
#define YELLOW 3

static int istty;

static int
puterr ( const int c );

static void
print_error ( const char *msg, va_list args );

void
error ( const char *msg, ... )
{
  va_list args;

  // imprime caracteres de escape para cores apenas se for para um terminal
  if ((istty = isatty(STDERR_FILENO)))
    {
      tputs ( exit_attribute_mode, 1, puterr );
      tputs ( tparm ( set_a_foreground, YELLOW ), 1, puterr );
      tputs ( enter_bold_mode, 1, puterr );
    }

  fprintf ( stderr, ERROR " " );

  if (istty)
    tputs ( exit_attribute_mode, 1, puterr );

  va_start ( args, msg );

  print_error ( msg, args );

  va_end ( args );
}

void
fatal_error ( const char *msg, ... )
{
  va_list args;

  // imprime caracteres de escape para cores apenas se for para um terminal
  if ((istty = isatty(STDERR_FILENO)))
    {
      tputs ( exit_attribute_mode, 1, puterr );
      tputs ( tparm ( set_a_foreground, RED ), 1, puterr );
      tputs ( enter_bold_mode, 1, puterr );
    }

  fprintf ( stderr, FATAL " " );

  if (istty)
    tputs ( exit_attribute_mode, 1, puterr );

  va_start ( args, msg );

  print_error ( msg, args );

  va_end ( args );
  exit ( EXIT_FAILURE );
}

// inclui '\n' no fim da mensagem e imprime
// na saida de erro padrão
static void
print_error ( const char *msg, va_list args )
{
  char *msg_formated;

  errno = 0;
  // inclui '\n' na mensagem
  if ( asprintf ( &msg_formated, "%s\n", msg ) == -1 )
    {
      if ( errno )  // asprintf set errno in case fault?
        perror ( "asprintf" );
      else
        fprintf ( stderr, "%s\n", "asprintf: Unknown error" );

      exit ( EXIT_FAILURE );
    }

  // exibe a mensagem
  vfprintf ( stderr, msg_formated, args );
  free ( msg_formated );
}

static int
puterr ( const int c )
{
  return fputc ( c, stderr );
}
