#ifndef TERMINAL_H
#define TERMINAL_H

// carrega os parametros do terminal
void
setup_terminal ( void );

// volta configurações original do terminal
void
restore_terminal ( void );

// limpa a tela, podendo tambem limpar o buffer do scroll se disponivel
void
clear_cmd ( void );

#endif  // TERMINAL_H
