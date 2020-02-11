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

#include "process.h"

bool debug = false;


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
// retorna o total de diretorios encontrados, -1 em caso de falha.
static int
get_numeric_directory(const char *process_dir, unsigned int *buffer, const int lenght)
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
static int
get_info_conections
(const char *conection_file, conection_t *conection, const int lenght)
{

  // uint32_t id;
  // uint32_t local_address;
  // int local_port;
  // uint32_t remote_address;
  // int remote_port;
  // uint8_t  con_state;
  // uint32_t tx_queue;
  // uint32_t rx_queue;
  // uint8_t  timer_active;
  // uint32_t tm_when;
  // uint32_t retrnsmt;
  // uint16_t uid;
  // uint8_t  timeout;
  // unsigned long inode;
  //

  FILE *arq = NULL;

  if ((arq = fopen(conection_file, "r")) == NULL)
    return 0;


  char *line = NULL;
  size_t len = 0;

  // ignore header in first line
  if ((getline(&line, &len, arq)) == -1)
    {
      free(line);
      return 0;
    }


  int count = 0;
  char local_addr[64], rem_addr[64] = {0};

  unsigned int matches, local_port, rem_port; // local_addr, rem_addr;
  unsigned long int inode;

  while ((getline(&line, &len, arq)) != -1 && (count < lenght))
    {

      // sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
      //  0: 0100007F:7850 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 19860 1 00000000d8098b1f 100 0 0 10 0
      //  1: 0100007F:78B4 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 19859 1 00000000bb21a0eb 100 0 0 10 0
      //  2: 9902A8C0:86CA BCC0D9AC:146C 01 00000000:00000000 02:00000CB1 00000000  1000        0 62034 2 0000000028d298f1 36 4 0 10 -1
      //  3: 9902A8C0:9A26 AB2B4E68:01BB 01 00000000:00000000 02:00000178 00000000  1000        1 2612130 2 0000000014dc9e6e 52 4 10 10 -1
      //  4: 9902A8C0:93A6 1A71528C:01BB 01 00000000:00000000 02:00000D3E 00000000  1000        0 2277146 2 000000001c1e11b6 46 4 27 10 -1
      //  5: 9902A8C0:C582 7CFD1EC0:01BB 01 00000000:00000000 02:000001E2 00000000  1000        0 2437598 2 00000000bd8b447a 46 4 27 10 -1

      // sscanf(line, "%u: %x:%hx %x:%hx %hhx %x:%x %hhu:%x %x %hx %hhu %d",
      //        &id, &local_address, &local_port, &remote_address, &remote_port,
      //        &con_state, &tx_queue, &rx_queue, &timer_active, &tm_when,
      //        &retrnsmt, &uid, &timeout, &inode);

      matches = sscanf(line, "%*d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %*X "
                       "%*X:%*X %*X:%*X %*X %*d %*d %lu %*512s\n",
                       local_addr, &local_port, rem_addr, &rem_port, &inode);

      if (matches != 5)
        {
          fprintf(stderr, "Unexpected buffer: '%s'\n", line);
          fclose(arq);
          return 0;
        }


      conection[count].local_address = local_addr;
      conection[count].local_port = local_port;
      conection[count].remote_address = rem_addr;
      conection[count].remote_port = rem_port;
      // conection[count].con_state = con_state;
      conection[count].inode = inode;
      // conection[count].id = id;

      count++;
    }

  free(line);
  fclose(arq);

  return count;
}

// armazena o nome do processo no buffer e retorna o tamanho do nome do processo
// até o primeiro espaço, não incluindo caracter \0.
static int
get_name_process(const int pid, char **buffer)
{
  char path_cmdline[MAX_NAME];
  snprintf(path_cmdline, MAX_NAME, "/proc/%d/cmdline", pid);


  FILE *arq = fopen(path_cmdline, "r");

  if (arq == NULL)
    {
      puts("erro abrir cmdline");
      return -1;
    }

  char line[MAX_NAME];
  if ((fgets(line, MAX_NAME, arq)) == NULL)
    {
      fclose(arq);
      return -1;
    }


  size_t len = strlen(line);

  line[len] = '\0';
  *buffer = malloc(len);

  // pega o nome somente até o primeiro espaço, se nao fica muito comprido
  size_t i;
  size_t j;
  for (i = 0, j = 0; i < len && line[i] != ' '; i++, j++)
    (*buffer)[i] = line[i];
  for (; i < len; i++)
    (*buffer)[i] = '\0';

  fclose(arq);
  return j;
}

// pega todos os fds do processo em /proc/$id/fd
// e armazena no buffer passado, retorna a quantidade de fd encontrado
static int get_all_fd_process(const int pid, unsigned int *buffer)
{
  char path[MAX_NAME];
  snprintf(path, MAX_NAME, "/proc/%d/fd/", pid);

  // if ((access(path, R_OK)) == -1)
  //   return -1;

  if (debug)
    printf("path process: %s\n", path);

  unsigned int total_fd_process = 0;
  unsigned int temp_buff[MAX_CONECTIONS] = {0};

  total_fd_process = get_numeric_directory(path, temp_buff, MAX_CONECTIONS);

  if (debug)
    printf("total_fd_process: %d\n", total_fd_process);

  if (total_fd_process == -1)
    return -1;

  for (size_t i = 0; i < total_fd_process; i++)
    buffer[i] = temp_buff[i];


  return total_fd_process;
}


void print_process(process_t *process, const int lenght)
{
  int tot_con;
  int j;
  for (int i = 0; i < lenght; i++)
    {

      // printf("pid %d\n", process[i].pid);
      // printf("name %s\n", process[i].name);

      printf("process pid      - %d\n"
             "process name     - %s\n"
             "tot fds          - %d\n"
             "total conections - %d\n",
              process[i].pid, process[i].name, process[i].total_fd,
              process[i].total_conections);
      printf("inodes           - ");

      j = 0;
      tot_con = process[i].total_conections;

      while(tot_con--)
        printf("%d ", process[i].conection[j++].inode);

      printf("\n\n");
    }
}


void free_process(process_t *process, const int lenght)
{
  for (size_t i = 0; i < lenght; i++) {
    free(process[i].name);
    process[i].name = NULL;
    free(process[i].conection);
    process[i].conection = NULL;
  }
  free(process);
  process = NULL;

}

int get_process_active_con(process_t **procs)
{
  int total_process = 0;
  unsigned int process_pids[MAX_PROCESS] = {0};
  // process_t process_con_act[100] = {0}; // processos com conexões ativas
  total_process = get_numeric_directory(PROCESS_DIR, process_pids, MAX_PROCESS);

  int total_conections = 0;
  conection_t conections[MAX_CONECTIONS] = {0};
  total_conections =  get_info_conections(PATH_INODE, conections, MAX_CONECTIONS);


  if (debug)
    {
      printf("total de process %d\n", total_process);
      printf("tam struct process %zu\n", sizeof(process_t));
    }

  process_t processos[total_process];
  memset(processos, 0, total_process);


  /*
   percorre todos os processos encontrados no diretório '/proc/',
   em cada processo encontrado armazena todos os file descriptors
   do processo - /proc/$id/fd - em process_t->fds,
   depois compara o link simbolico apontado pelo FD com 'socket:[inode]',
   sendo inode coletado do arquivo '/proc/net/tcp', caso a comparação seja igual,
   encontramos o processo que corresponde ao inode (conexão).
  */
  char path_fd[MAX_NAME] = {0};         // proc/pid/fd/
  char socket[MAX_NAME_SOCKET] = {0};
  char data_fd[MAX_NAME_SOCKET] = {0};
  uint32_t fds_p[MAX_CONECTIONS] = {0};
  int len_link = 0;

  int total_fd_process = 0;


  bool process_have_conection_active;
  int process_active_conection = 0;
  int tmp_tot_fd = 0;;

  int index_conection;
  int index_history_con[total_conections];

  for (int pd = 0; pd < total_process; pd++) // pd - process pid
    {

      process_have_conection_active = false;

      index_conection = 0;
      memset(index_history_con, 0, total_conections);

      total_fd_process = get_all_fd_process(process_pids[pd], fds_p);


      if (debug)
        printf("total_fd_process %d\n", total_fd_process);

      // troca de processo
      if (total_fd_process < 0)
        continue;

      tmp_tot_fd = total_fd_process;

      while (tmp_tot_fd)
        {
          // // isola um fd por vez do processo
          snprintf(path_fd, 100, "/proc/%d/fd/%d", process_pids[pd], fds_p[tmp_tot_fd--]);

          if (debug)
            printf("path_fd %s\n", path_fd);

          //
          // // se der erro para ler, provavel acesso negado
          // // vai pro proximo fd do processo
          if ((len_link = readlink(path_fd, data_fd, 1000)) == -1)
            continue;

          //
          data_fd[len_link] = '\0';

          //
          // // caso o link nao tenha a palavra socket
          // // vai para proximo fd do processo
          if (!strstr(data_fd, "socket"))
            continue;

          // compara o fd do processo com todos os inodes disponiveis
          for (int c = 0; c < total_conections; c++)
            {
              // connection is in TIME_WAIT state, test next conection
              if (conections[c].inode == 0)
                continue;

              snprintf(socket, MAX_NAME_SOCKET, "socket:[%d]", conections[c].inode);

              if (debug)
                printf("socket %s\n", socket);

              // se o conteudo de socket - socket:[$inode] - for igual
              // ao valor lido do fd do processo,
              // encontrado de qual processo a conexão pertence
              if ((strncmp(socket, data_fd, len_link)) == 0)
                {
                  // salva o indice do array conections que tem a conexao
                  // do processo para depois pegar os dados desses indices
                  process_have_conection_active = true;
                  index_history_con[index_conection++] = c;
                }

            }
        } // while fd


      if (process_have_conection_active)
        {

          // obtem informações do processo
          processos[process_active_conection].pid = process_pids[pd];
          get_name_process(process_pids[pd], &processos[process_active_conection].name);
          processos[process_active_conection].total_fd = total_fd_process;
          processos[process_active_conection].total_conections = index_conection;

          //pega as conexões ativas no array conections referentes ao processo
          //e adiciona a ele
          processos[process_active_conection].conection = malloc(sizeof(conection_t) * processos[process_active_conection].total_conections);

          for (int c = 0; c < index_conection; c++)
            {
              if (debug)
                printf("associando ao processo pid %d o inode %d\n",
                      processos[process_active_conection].pid, conections[index_history_con[c]].inode);
                // printf("index_histopry[%d] - %d\n", index_conection - c - 1, index_history_con[c])

              processos[process_active_conection].conection[c] = conections[index_history_con[c]];

              if (debug)
              printf("associado ao processo pid %d o inode %d - index %d\n",
                    processos[process_active_conection].pid, processos[process_active_conection].conection[c].inode, c);
            }

          // contabiliza total de processos que possuem conexao ativa
          process_active_conection++;
        }

    } //for process

  // sem processos com conexao ativa, retorna 0
  if (!process_active_conection){
    *procs = NULL;
    return process_active_conection;
  }

  *procs = malloc(sizeof(process_t) * process_active_conection);

  for (int i = 0; i < process_active_conection; i++)
       (*procs)[i] = processos[i];

  // retorna o numero de processos com conexão ativa
  return process_active_conection;

}
