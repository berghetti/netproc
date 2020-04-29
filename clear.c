
#include <stdio.h>    // putchar
#include <unistd.h>   // STDOUT_FILENO
#include <term.h>     // setupterm, putp

#include "m_error.h"


// carrega informações do terminal a associa a stdout
void load_terminal(void)
{
  int err;
  setupterm(NULL, STDOUT_FILENO, &err);

  switch (err)
    {

  	case -2:
  		fatal_error("unreadable terminal descriptor");
  		break;
  	case -1:
      fatal_error("no terminfo database");
  		break;
  	case 0:
      fatal_error("unknown terminal");

  	}
}

// imprime caracter fornecido por tputs
static int
putch(int c)
{
    return putchar(c);
}

// limpa a tela, podendo tambem limpar o buffer do scroll
int
clear_cmd(int clear_scroll)
{
    if (clear_scroll)
      {
        char *E3 = tigetstr("E3");
        if (E3)
          (void) tputs(E3, lines > 0 ? lines : 1, putch);
      }

    return tputs(clear_screen, lines > 0 ? lines : 1, putch);
}
