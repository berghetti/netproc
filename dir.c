#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>

#include <dirent.h>
#include <ctype.h>

#define MAX_PROCESS 1024
#define PROCESS_DIR "/proc/"

#define MAX_INODES 1024
#define MAX_CONECTIONS 1024
#define PATH_INODE "/proc/net/tcp"

#define MAX_FD 1024

#define MAX_NAME 1024

bool debug = false;


typedef struct
{
  uint32_t id;
  uint32_t local_address;
  uint16_t local_port;
  uint32_t remote_address;
  uint16_t remote_port;
  uint8_t  con_state;
  uint32_t inode;
}conection_t;

typedef struct
{
  char name[MAX_NAME];          // nome do processo
  conection_t conections[MAX_CONECTIONS];      // conexoes do processo
  unsigned int pid;             // pid do processo
  unsigned int total_fd;          // totalal de fd no processo
  unsigned int total_conections; // total de conexoes do processo
  unsigned int *fds;            // todos os file descriptos - /proc/pid/fd

}process_t;


static bool is_number(const char *caracter)
{
  while (*caracter)
    {
      if (!isdigit(*caracter))
        return false;

      caracter++;
    }

  return true;
}


// recebe o nome do diretorio dos processos, um buffer para armazenar
// os nomes dos diretorios e um tamanho maximo para o buffer.
// retorna o totalal de diretorios encontrados, -1 em caso de falha.
int get_numeric_directory(const char *process_dir, unsigned int *buffer, const int lenght)
{
  DIR *dir;
  if ((dir = opendir(process_dir)) == NULL)
    return -1;

  struct dirent *directory = NULL;
  int count = 0;
  while ((directory = readdir(dir)) != NULL && count < lenght)
    {
      if (is_number(directory->d_name))
        {
          buffer[count] = atoi(directory->d_name);
          count++;
        }

    }

  closedir(dir);

  return count;
}


// le o arquivo onde fica salva as conexoes '/proc/net/tcp',
// recebe o local do arquivo, um buffer para armazenar
// dados da conexão e o tamanho do buffer,
// retorna a quantidade de registros encontrada
// ou zero em caso de erro
int
get_info_conections
(const char *inode_file, conection_t *conection, const int lenght)
{

  uint32_t id;
  uint32_t local_address;
  uint16_t local_port;
  uint32_t remote_address;
  uint16_t remote_port;
  uint8_t  con_state;
  uint32_t tx_queue;
  uint32_t rx_queue;
  uint8_t  timer_active;
  uint32_t tm_when;
  uint32_t retrnsmt;
  uint16_t uid;
  uint8_t  timeout;
  uint32_t inode;

  FILE *arq;
  if ((arq = fopen(inode_file, "r")) == NULL)
    return 0;

  char *line;
  size_t len = 0;
  // ignore header in first line
  if ((getline(&line, &len, arq)) == -1)
    return 0;

  ssize_t nread;
  int count = 0;
  while ((nread = getline(&line, &len, arq)) != -1 && count < lenght)
    {
      sscanf(line, "%d: %x:%x %x:%x %x %x:%x %d:%x %x %x %d %d",
             &id, &local_address, &local_port, &remote_address, &remote_port,
             &con_state, &tx_queue, &rx_queue, &timer_active, &tm_when,
             &retrnsmt, &uid, &timeout, &inode);


      conection[count].local_address = local_address;
      conection[count].local_port = local_port;
      conection[count].remote_address = remote_address;
      conection[count].remote_port = remote_port;
      conection[count].con_state = con_state;
      conection[count].inode = inode;
      conection[count].id = id;

      count++;
    }

  return count;
}


int get_name_process(const int pid, char *buffer)
{
  char path_cmdline[MAX_NAME];
  snprintf(path_cmdline, MAX_NAME, "/proc/%d/cmdline", pid);


  FILE *arq = fopen(path_cmdline, "r");
  if (arq == NULL){
    puts("erro abrir cmdline");
    return -1;
  }

  char line[MAX_NAME];
  if ((fgets(line, MAX_NAME, arq)) == NULL)
    return -1;

  strcpy(buffer, line);

  size_t len = strlen(buffer);

  if (debug)
    printf("get_name_process: len - %d\n", len);

  return len;

}

int get_fd_process(const int pid, unsigned int **buffer)
{
  char path[MAX_NAME];
  snprintf(path, MAX_NAME, "/proc/%d/fd/", pid);

  if (debug)
    printf("get_fd_process %s\n", path);

  unsigned int total_fd_process;
  unsigned int temp_buff[MAX_FD] = {0};

  total_fd_process = get_numeric_directory(path, temp_buff, MAX_FD);
  temp_buff[total_fd_process] = 0;

  if (debug)
    printf("total_fd_process: %d\n", total_fd_process);

  *buffer = malloc(total_fd_process);
  *buffer = temp_buff;

  return total_fd_process;
}


void get_info_pid(const int pid, process_t *processo)
{
  char test_process[MAX_NAME];
  snprintf(test_process, MAX_NAME, "/proc/%d/fd/", pid);

  // return case not access
  if ((access(test_process, R_OK)) == -1)
    return;

  processo->pid = pid;
  get_name_process(pid, processo->name);
  processo->total_fd = get_fd_process(pid, &processo->fds);

}


int main(void)
{
  int total_process;
  unsigned int process_pids[MAX_PROCESS] = {0};
  process_t process_con_act[1000] = {0}; // processos com conexões ativas
  total_process = get_numeric_directory(PROCESS_DIR, process_pids, MAX_PROCESS);

  int total_conections;
  conection_t conections[MAX_CONECTIONS] = {0};
  total_conections =  get_info_conections(PATH_INODE, conections, MAX_CONECTIONS);

  unsigned int total_proc_con_act = 0;


  if (debug)
    printf("tam struct process %d\n", sizeof (process_t));

  process_t *processos =  malloc(sizeof(process_t) * total_process);
  memset(processos, 0, sizeof(process_t) * total_process);



  //get_info_pid(process_pids[140], &processos[0]);

  if (debug){
    printf("process pid:    %d\n", processos[0].pid);
    printf("process name:   %s\n", processos[0].name);
    printf("total fd process: %d\n", processos[0].total_fd);
    printf("process fds: ");
    while (processos[0].total_fd){
      printf("%d ", processos[0].fds[processos[0].total_fd--]);
    }
    putchar('\n');
  }

  /*
   percorre todos os processos encontrados no diretório '/proc/',
   em cada processo encontrado armazena todos os file descriptors do processo em fds_process_buff,
   depois compara o link simbolico apontado pelo FD com 'socket:[inode]', sendo inode coletado
   do arquivo '/proc/net/tcp', caso a comparação seja igual, encontramos o processo que corresponde
   ao inode (conexão).
  */
  char path_fd[100] = {0};
  char socket[100] = {0};
  char temp_buff[1000] = {0};
  int len_link = 0;
  bool check_con;
  for (int i = 0; i < total_process; i++)
    {
      // true if process active conection
      check_con = false;

      // pega todos os file descriptors dos processos encontrados em '/proc/$id/fd'
      // snprintf(process, 100, "/proc/%d/fd/", process_pids[i]);
      // total_fd_in_process = get_numeric_directory(process, fds_process_buff, 1000);

      // get information process
      get_info_pid(process_pids[i], &processos[i]);

      while (processos[i].total_fd)
        {
          // isola um fd por vez do processo
          snprintf(path_fd, 100, "/proc/%d/fd/%d", processos[i].pid, processos[i].fds[--processos[i].total_fd]);

          // se der erro para ler, troca de processo
          if ((len_link = readlink(path_fd, temp_buff, 1000)) == -1)
            break;

          temp_buff[len_link] = '\0';

          // compara o fd do processo com todos os inodes disponiveis
          for (int j = 0; j < total_conections; j++)
            {
              snprintf(socket, 100, "socket:[%d]", conections[j].inode);

              // se o conteudo de socket - socket:[$inode] - for igual
              // ao valor lido do fd do processo,
              // encontrado de qual processo o inode corresponde
              if (!strncmp(socket, temp_buff, len_link))
              {
                check_con = true;
                // adiciona ao processo dados de sua conexao e incrementa a quantidade de conexoes
                // processos[i].conections[processos[i].total_conections++] = conections[j];

                // process_con_act[total_proc_con_act].pid = processos[i].pid;
                // strcpy(process_con_act[total_proc_con_act].name, processos[i].name);
                // process_con_act[total_proc_con_act].total_conections++;
                // process_con_act[total_proc_con_act].conections[process_con_act[total_proc_con_act].total_conections] = conections[j];

                printf("process pid - \t%d\nprocess name \t%s\ninode - \t%d\n",
                        processos[i].pid, processos[i].name, conections[j].inode);

                // printf("total de conexões - %d\n", processos[i].total_conections);
              }

            }
        }
        // incrementa quando troca de processo,
        // se teve processo com conexao ativa
        // if (check_con) total_proc_con_act++;

    }

    printf("\n\n\n");

    // for(int i = 0; i < total_proc_con_act; i++)
    // {
    //
    //   printf("total_proc_con_act %d\n", total_proc_con_act);
    //
    //   printf("id: \t%d\n", process_con_act[i].pid);
    //   printf("name: \t%s\n", process_con_act[i].name);
    //   printf("tot_con: \t%d\n", process_con_act[i].total_conections);
    //
    // }

}
