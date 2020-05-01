
#include <stdio.h>    // putchar
#include <unistd.h>   // STDOUT_FILENO
#include <term.h>     // setupterm, tputs, tigetstr

#include "m_error.h"


static char *E3;

// carrega informações do terminal a associa a stdout
void setup_terminal(void)
{
  int err;
  setupterm(NULL, STDOUT_FILENO, &err);

  // codigo de escape para limpar scrollback
  E3 = tigetstr("E3");

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

  // tputs(cursor_invisible, 1, putchar);
}


// limpa a tela, podendo tambem limpar o buffer do scroll se disponivel
// Obs: não alterar ordem entre limpar a tela e limpar scroll
void
clear_cmd(void)
{
  // limpa a tela
  tputs(clear_screen, lines > 0 ? lines : 1, putchar);

  // se recurso para limpar scroll estiver disponivel
  if (E3)
    tputs(E3, lines > 0 ? lines : 1, putchar);

}
