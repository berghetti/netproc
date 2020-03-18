#ifndef PROCESS_H
#define PROCESS_H

#include <ctype.h>

#define MAX_PROCESS 13504 // ulimit -a
#define PROCESS_DIR "/proc/"

#define MAX_CONECTIONS 1024
#define PATH_INODE "/proc/net/tcp"

#define MAX_NAME_SOCKET 9 + 8 + 3 // socket:[99999999] + 3 safe
#define MAX_NAME 1000

typedef struct
{
  // uint32_t id;
  // struct in_addr
  uint32_t local_address;
  uint16_t local_port;
  uint32_t remote_address;
  uint16_t remote_port;
  // uint8_t  con_state;
  uint32_t inode;
}conection_t;

typedef struct
{
  conection_t *conection;     // conexoes do processo
  char *name;                  // nome processo
  // uint32_t *fds;               // todos os file descriptos - /proc/pid/fd
  uint32_t pid;                // pid do processo
  uint32_t total_fd;           // totalal de fd no processo
  uint32_t total_conections;   // total de conexoes do processo
}process_t;


// inicializa a estrutura process_t para o endereço onde estão
// armazenados os processos ativos e retorna a quantidade processos ativos
int get_process_active_con(process_t **procs);


void print_process(process_t *process, const int lenght);

// percerre a array e os libera
void free_process(process_t *process, const int lenght);




#endif // PROCESS_H
