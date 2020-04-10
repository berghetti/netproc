#ifndef MTIME_H
#define MTIME_H

#include "headers-system.h"


// inicializa o timer com a hora atual
void init_timer(void);

// retorna a diferença da hora inicializada
// com a hora atual
float timer(void);

// atribui a hora atual como hora de inicio
void restart_timer(void);


#endif // MTIME_H
