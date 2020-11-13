
/*
 *  Copyright (C) 2020 Mayco S. Berghetti
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROCESS_H
#define PROCESS_H

#include "config.h"
#include "conection.h"
#include "directory.h"
#include "rate.h"

// FIXME:
// ulimit -a do meu sistema, melhorar isso...
// #define MAX_PROCESS 13504

// diretorios onde são listados os processos do sistema
#define PROCESS_DIR "/proc/"

// sizeof ("socket:[99999999]") + 3 safe
#define MAX_NAME_SOCKET 9 + 8 + 3

// tamanho maximo do nome de um processo
#define MAX_NAME 255

// maxpid = 2^22 = 4194304 = 7 chars
//   6  +  7  + 4 + 7
// /proc/<pid>/fd/<id-fd>
#define MAX_PATH_FD 24

typedef struct process
{
  struct net_stat net_stat;   // estatisticas de rede
  conection_t *conection;     // array de conexoes do processo
  char *name;                 // nome processo
  pid_t pid;                  // pid do processo
  uint32_t total_conections;  // total de conexões apontada por conection_t *

  // variavel de controle, armazena o numero maximo
  // de conexoes que podem ser armazenada no array *conection_t antes
  // que a memoria precise ser realocada
  uint32_t max_n_con;
} process_t;

// inicializa a estrutura process_t com os processos ativos e
// retorna a quantidade processos armazenados
int
get_process_active_con ( process_t **procs,
                         const size_t tot_process_act_old,
                         const struct config_op *co );

// libera os processos informados (usado para apagar todos os processos)
void
free_process ( process_t *proc, const size_t qtd_proc );

size_t
strlen_space ( const char *string );

#endif  // PROCESS_H
