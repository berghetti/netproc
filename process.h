#ifndef PROCESS_H
#define PROCESS_H


#include "directory.h"
#include "conection.h"



// ulimit -a do meu sistema, melhorar isso...
#define MAX_PROCESS 13504

// diretorios onde são listados os processos do sistema
#define PROCESS_DIR "/proc/"

// sizeof ("socket:[99999999]") + 3 safe
#define MAX_NAME_SOCKET 9 + 8 + 3

// tamanho maximo do nome de um processo
#define MAX_NAME 128

// maxpid = 2^22 = 4194304 = 7 chars
//   6  +  7  + 4 + 7
// /proc/<pid>/fd/<id-fd>
#define MAX_PATH_FD 24

// espaço amostral para calcular a média
// de estatisticas de rede.
// 5 é um bom valor...
#define LEN_BUF_CIRC_RATE 5

// Considerando que a cada 1024 bits ou bytes (bits por segundo ou bytes por segundo),
// caso escolhido o padrão IEC com base 2,
// ou 1000 bits/bytes caso escolhido o padrão SI, com base 10,
// o valor sera dividido por 1000 ou 1024 para que
// possa ser apresentado de forma "legivel por humanos",
// assim sempre teremos algo como:
// 1023 B/s, 1023.99 KB/s, 1023.99 Mib/s, 1023.99 Gib/s, ou 8388608 TiB/s :o
// então no pior caso teremos umas string com tamanhao de 14 bytes ja incluido
// null byte.
#define LEN_STR_RATE 14

typedef uint64_t nstats_t;

struct net_stat
{
  char rx_rate[LEN_STR_RATE];
  char tx_rate[LEN_STR_RATE];
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
