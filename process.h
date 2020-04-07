#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#include <dirent.h>
#include <ctype.h>


#define MAX_PROCESS 13504 // ulimit -a

// diretorios onde são listados os processos do sistema
#define PROCESS_DIR "/proc/"

#define MAX_CONECTIONS 1024
#define PATH_INODE "/proc/net/tcp"

// sizeof (socket:[99999999]) + 3 safe
#define MAX_NAME_SOCKET 9 + 8 + 3
#define MAX_NAME 1024

// maxpid = 2^22 = 4194304 = 7 chars
//   6  +  7  + 4 + 7
// /proc/<pid>/fd/<id-fd>
#define MAX_PATH_FD 24

// tempo amostral para calcular a media
// de estatisticas de rede.
// 5 é um bom valor...
#define LEN_BUF_CIRC_RATE 5


struct net_stat
{
  uint32_t pps_rx[LEN_BUF_CIRC_RATE];
  uint32_t pps_tx[LEN_BUF_CIRC_RATE];
  uint32_t Bps_rx[LEN_BUF_CIRC_RATE];
  uint32_t Bps_tx[LEN_BUF_CIRC_RATE];
  uint32_t avg_Bps_rx;
  uint32_t avg_Bps_tx;
  uint32_t avg_pps_rx;
  uint32_t avg_pps_tx;
};

typedef struct
{
  // uint32_t id;
  // struct in_addr

  uint32_t inode;
  uint32_t local_address;
  uint32_t remote_address;
  uint16_t local_port;
  uint16_t remote_port;
  // uint8_t  con_state;


}conection_t;

typedef struct
{
  struct net_stat net_stat;     // estatisticas de rede
  conection_t *conection;       // conexoes do processo
  char *name;                   // nome processo
  // uint32_t *fds;             // todos os file descriptos - /proc/pid/fd
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
