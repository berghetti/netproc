#ifndef M_TIME_H
#define M_TIME_H

// para facilitar a leitura do codigo
#define restart_timer start_timer

// retorna a diferença entre o tempo atual em segundos
// e o valor em segundos passado por parametro
double timer(const float old_time);

// retorna a tempo atual em segundos
double start_timer(void);

#endif // MTIME_H
