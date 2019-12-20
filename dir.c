#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include <dirent.h>
#include <ctype.h>

#define MAX_PROCESS 1024
#define PROCESS_DIR "/proc/"

#define MAX_INODES 1024
#define PATH_INODE "/proc/net/tcp"

#define MAX_FD 50000

struct process{
  int pid;
  int fd[MAX_FD];
};


int get_all_fd_process(const char *process, int *buffer, const int len)
{
  DIR *dir;
  struct dirent *directory = NULL;
  int count = 0;
  if ( (dir = opendir(process)) == NULL )
  return 0;

  while((directory = readdir(dir)) != NULL)
  {
    if (isdigit(directory->d_name[0]))
    {
      // printf("%s\n", directory->d_name);
      buffer[count] = atoi(directory->d_name);
      count++;
    }
  }

  return count;
}


// recebe o nome do diretorio dos processos, um buffer para armazenar
// os PIDs e um tamanho maximo para o buffer.
// retorna o total de processos ativos, 0 em caso de falha.
int get_pid_all_process(const char *process_dir, int *buffer, const int lenght)
{
  DIR *dir;
  struct dirent *directory = NULL;
  int count = 0;

  if ((dir = opendir(process_dir)) == NULL)
  return 0;

  while ((directory = readdir(dir)) != NULL && count < lenght)
  {
    if (isdigit(directory->d_name[0]))
    {
      buffer[count] = atoi(directory->d_name);
      count++;
    }
  }

  return count;

}


// le o arquivo onde fica salva as conexoes
int get_inodes(const char *inode_file, unsigned int *buffer, const int lenght)
{

  FILE *arq;
  char *line;
  size_t len = 0;
  ssize_t nread;

  int count = 0;

  unsigned int id,
              local_address,
              local_port,
              remote_address,
              remote_port,
              con_state,
              tx_queue,
              rx_queue,
              timer_active,
              tm_when,
              retrnsmt,
              uid,
              timeout,
              inode;


  if ((arq = fopen(inode_file, "r")) == NULL)
  return 0;

  // ignore first line
  if ((getline(&line, &len, arq)) == -1)
  return 0;

  while ((nread = getline(&line, &len, arq)) != -1 && count < lenght)
  {
    sscanf(line, "%d: %x:%x %x:%x %x %x:%x %d:%x %x %x %d %d",
    &id, &local_address, &local_port, &remote_address, &remote_port,
    &con_state, &tx_queue, &rx_queue, &timer_active, &tm_when,
    &retrnsmt, &uid, &timeout, &inode);

    //printf("local addres - %d \tinode - %d\n", local_address, inode);

    buffer[count] = inode;
    count++;
  }

  return count;
}



int main(void)
{

  int tot_process;
  int process_pids[MAX_PROCESS] = {0};

  tot_process = get_pid_all_process(PROCESS_DIR, process_pids, MAX_PROCESS);


  int tot_inodes;
  unsigned int inodes[MAX_INODES] = {0};

  tot_inodes =  get_inodes(PATH_INODE, inodes, MAX_INODES);


  int tot_fd_in_process = 0;
  int fds_buff[1000];
  char path_fd[100];
  char process[100];
  char socket[100];

  char temp_buff[1000];
  ssize_t len_link;
  for (int i = 0; i < tot_process; i++)
  {
    // puts("aio");
    snprintf(process, 100, "/proc/%d/fd/", process_pids[i]);
    //printf("%s\n", process);
    tot_fd_in_process = get_all_fd_process(process, fds_buff, 1000);

    while(tot_fd_in_process)
    {
      snprintf(path_fd, 100, "/proc/%d/fd/%d", process_pids[i], fds_buff[tot_fd_in_process -1]);
      for (int j = 0; j < tot_inodes; j++)
      {
        snprintf(socket, 100, "socket:[%d]", inodes[j]);
        // printf("%s\n", socket);
        len_link = readlink(path_fd, temp_buff, 99);
        temp_buff[len_link] = '\0';

        if (!strncmp(socket, temp_buff, len_link))
          printf("%s\n%s\n\n", socket, temp_buff);
      }

       // printf("%s\n", path_fd);
      tot_fd_in_process--;
    }

  }

  // for (int j = 0; j < tot_inodes; j++)
  // {
  //   printf("inode - %d\n", inodes[j]);
  //   // get_all_fd_process("/proc/2041/fd/");
  //
  //
  // }


}
