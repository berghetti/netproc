#ifndef PROCESS_H
#define PROCESS_H

#include "headers-system.h"

#include "directory.h"
#include "conection.h"


#define MAX_PROCESS 13504 // ulimit -a

// diretorios onde são listados os processos do sistema
#define PROCESS_DIR "/proc/"

// checar isso...
#define MAX_CONECTIONS 1024

// caminho das conexoes TCP, estender isso....
#define PATH_INODE "/proc/net/tcp"

// sizeof (socket:[99999999]) + 3 safe
#define MAX_NAME_SOCKET 9 + 8 + 3

// tamanho maximo do nome de um processo
#define MAX_NAME 128

// maxpid = 2^22 = 4194304 = 7 chars
//   6  +  7  + 4 + 7
// /proc/<pid>/fd/<id-fd>
#define MAX_PATH_FD 24

// espaço amostral para calcular a media
// de estatisticas de rede.
// 5 é um bom valor...
#define LEN_BUF_CIRC_RATE 5

typedef uint32_t nstats_t;

struct net_stat
{
  nstats_t pps_rx[LEN_BUF_CIRC_RATE];
  nstats_t pps_tx[LEN_BUF_CIRC_RATE];
  nstats_t Bps_rx[LEN_BUF_CIRC_RATE];
  nstats_t Bps_tx[LEN_BUF_CIRC_RATE];
  nstats_t avg_Bps_rx;
  nstats_t avg_Bps_tx;
  nstats_t avg_pps_rx;
  nstats_t avg_pps_tx;
};

typedef struct
{
  struct net_stat net_stat;     // estatisticas de rede
  conection_t *conection;       // conexoes do processo
  char *name;                   // nome processo
  pid_t pid;                    // pid do processo
  uint32_t total_fd;            // totalal de fd no processo
  uint32_t total_conections;    // total de conexões apontada por conection_t *

  // variavel de controle, armazena o numero maximo
  // de conexoes que podem ser armazenada antes
  // que a memoria precise ser realocada
  uint32_t max_n_con;
}process_t;


// inicializa a estrutura process_t para o endereço onde estão
// armazenados os processos ativos e retorna a quantidade processos ativos
int get_process_active_con(process_t **procs, const size_t tot_process_act_old);


// int refresh_process_active_con(process_t **old_process, const int tot_old);


void print_process(process_t *process, const int lenght);

// percerre a array e os libera
// void free_process(process_t *process, const int lenght);




#endif // PROCESS_H
