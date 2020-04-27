#ifndef M_TIME_H
#define M_TIME_H

// para facilitar a leitura do codigo
#define restart_timer start_timer

// // inicializa o timer com a hora atual
// void init_timer(void);
//
// // retorna a diferença da hora inicializada
// // com a hora atual
// float timer(void);
//
// // atribui a hora atual como hora de inicio
// void restart_timer(void);

// retorna a diferença entre o tempo atual em segundos
// e o valor em segundos passado por parametro
float timer(float old_time);

// retorna a tempo atual em segundos
float start_timer(void);

#endif // MTIME_H
