#ifndef CLEAR_H
#define CLEAR_H

// limpa a tela, podendo tambem limpar o buffer do scroll
int clear_cmd(int clear_scroll);

// carrega os parametros do terminal
void load_terminal(void);

#endif // CLEAR_H
