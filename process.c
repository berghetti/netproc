
#include "process.h"

bool debug = false;


static bool is_number(const char *string)
{
  while (*string)
      if (! isdigit(*string++))
        return false;

  return true;
}


// recebe o nome do diretorio dos processos, um buffer para armazenar
// os nomes dos diretorios e um tamanho maximo do buffer.
// retorna o total de diretorios encontrados, -1 em caso de falha.
static int
get_numeric_directory(uint32_t *buffer,
                      const size_t lenght,
                      const char *path_dir)
{
  DIR *dir;

  if ((dir = opendir(path_dir)) == NULL)
    return -1;

  struct dirent *directory = NULL;
  uint32_t count = 0;

  while ( (directory = readdir(dir)) && count < lenght )
      if (is_number(directory->d_name))
          buffer[count++] = atoi(directory->d_name);


  closedir(dir);

  return count;
}


// le o arquivo onde fica salva as conexoes '/proc/net/tcp',
// recebe o local do arquivo, um buffer para armazenar
// dados da conexão e o tamanho do buffer,
// retorna a quantidade de registros encontrada
// ou -1 em caso de erro
static int
get_info_conections(conection_t *conection,
                    const size_t lenght,
                    const char *conection_file)
{

  FILE *arq = NULL;

  if (!(arq = fopen(conection_file, "r")))
    return -1;


  char *line = NULL;
  size_t len = 0;

  // ignore header in first line
  if ((getline(&line, &len, arq)) == -1)
    {
      free(line);
      line = NULL;
      fclose(arq);
      return -1;
    }


  uint32_t count = 0;
  char local_addr[64], rem_addr[64] = {0};

  unsigned int matches, local_port, rem_port; // local_addr, rem_addr;
  unsigned long int inode;

  while ( (getline(&line, &len, arq)) != -1 && (count < lenght) )
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
          return -1;
        }


      if (strlen(local_addr) == 8) // only ipv4
        {
          // converte char para tipo inteiro
          if (1 != sscanf(local_addr, "%x", &conection[count].local_address))
            perror("sscanf");

          if (1 != sscanf(rem_addr, "%x", &conection[count].remote_address))
            perror("sscanf");

          conection[count].local_port = local_port;
          conection[count].remote_port = rem_port;
          // conection[count].con_state = con_state;
          conection[count].inode = inode;
          // conection[count].id = id;

          count++;
        }

      }

  free(line);
  line = NULL;
  fclose(arq);

  return count;
}

// retorna o tamanho da string até null byte ou espaço
// oque ocorrer primeiro
static size_t
strlen_space(const char *string)
{
  size_t n = 0;
  while(*string && *string++ != ' ')
    n++;

  return n;
}

// armazena o nome do processo no buffer e retorna
// o tamanho do nome do processo incluindo null bytes ou espaço
// função cuida da alocação de memoria para o nome do processo
static int
get_name_process(char **buffer, const pid_t pid)
{
  char path_cmdline[MAX_NAME];
  snprintf(path_cmdline, MAX_NAME, "/proc/%d/cmdline", pid);


  FILE *arq = NULL;
  arq = fopen(path_cmdline, "r");

  if (arq == NULL)
    {
      perror("fopen");
      return -1;
    }

  char line[MAX_NAME];
  if ( ! fgets(line, MAX_NAME, arq) )
    {
      fclose(arq);
      perror("fgets");
      return -1;
    }

  // tamanho até null byte ou primeiro espaço
  size_t len = strlen_space(line);

  line[len] = '\0';

  *buffer = calloc(1, len + 1);

  if (! buffer)
    {
      perror("calloc");
      return -1;
    }

  // copia a string junto com null byte
  size_t i;
  for (i = 0; i < len + 1; i++)
    (*buffer)[i] = line[i];


  fclose(arq);
  return i;
}


void print_process(process_t *process, const int lenght)
{
  int tot_con;
  int j;
  char buffer_ip[INET_ADDRSTRLEN];
  for (int i = 0; i < lenght; i++)
    {

      printf("process pid      - %d\n"
             "process name     - %s\n"
             "tot fds          - %d\n"
             "total conections - %d\n",
              process[i].pid, process[i].name, process[i].total_fd,
              process[i].total_conections);
      printf("conections       -\n");

      j = 0;
      tot_con = process[i].total_conections;

      while(tot_con--){
        if (inet_ntop(AF_INET, &process[i].conection[j].local_address, buffer_ip, INET_ADDRSTRLEN) != NULL)
        printf("%s:", buffer_ip);
        printf("%d <--> ", process[i].conection[j].local_port);

        if (inet_ntop(AF_INET, &process[i].conection[j].remote_address, buffer_ip, INET_ADDRSTRLEN) != NULL)
        printf("%s:", buffer_ip);
        printf("%d ", process[i].conection[j].remote_port);

        printf("\t%d", process[i].conection[j].inode);
        j++;
        putchar('\n');
      }

      printf("\n\n");

    }
}

// liebra processos atuais que não foram localizados
// na ultima checagem por processos ativos
void free_process(process_t *cur_procs,
                  const size_t len_cur_procs,
                  const process_t *new_procs,
                  const size_t len_new_procs)
{

  bool locate;
  for (size_t i = 0; i < len_cur_procs; i++)
    {
      locate = false;
      for (size_t j = 0; j < len_new_procs; j++)
        {
          if (cur_procs[i].pid == new_procs[j].pid)
            {
              locate = true;
              break;
            }
        }

      if (! locate)
        {
          free(cur_procs[i].name);
          cur_procs[i].name = NULL;
          free(cur_procs[i].conection);
          cur_procs[i].conection = NULL;
        }
    }

}

// aloca memoria INICIAL para conection da estrutura process_t
// com base no numero de conexoes que o processo tem,
// sendo numero de conexoes * 2 o tamanho alocado.
// Antes de reallocar mais memoria é feita verificações para checar se o valor
// alocado inicialmente não atende, caso não realocamos para nova quantidade
// de conexões * 2;
static void
alloc_memory_conections(process_t *new_st_processes,
                        const process_t *current_st_processes)
{

  if (! current_st_processes)
    { // processo novo, aloca duas vezes quantidade de memoria necessaria
      // para evitar realocar com frequencia...

      new_st_processes->conection = calloc(sizeof(conection_t),
                                        new_st_processes->total_conections * 2);

      if (!new_st_processes->conection)
        {
          perror("calloc");
          exit(EXIT_FAILURE);
        }

      new_st_processes->max_n_con = new_st_processes->total_conections * 2;
    }
  else if ( current_st_processes->max_n_con <
            new_st_processes->total_conections )
    { // status atual do processo nao tem memoria suficiente
      //  para armazenas a quantidade de conexões do novo status processo
      // realoca memoria para o dobro da nova demanda...

      void *p = NULL;
      p = realloc( current_st_processes->conection,
                  new_st_processes->total_conections * sizeof(conection_t) * 2);

      if (! p)
        {
          perror("realloc deu ruim");
          exit(EXIT_FAILURE);
        }

      new_st_processes->conection = p;
      new_st_processes->max_n_con = new_st_processes->total_conections * 2;

    }
  else
    {  // apenas reutiliza a memoria ja alocada, espaço é suficiente...
       new_st_processes->conection = current_st_processes->conection;
       new_st_processes->max_n_con = current_st_processes->max_n_con;
    }

}


// verifica se o processo ja existia na lista de processos
// com conexões ativas.
// returna o indice do processo do array de processos corrente
// que corresponde ao novo processo candidato, ou nao, caso seja um processo
// realmente novo, nesse caso retorna -1
static int
process_already_existed(process_t **current_procs,
                        const size_t tot_proc_current,
                        const pid_t new_proc_pid)
{
  if (! *current_procs)
    return -1;

  for (size_t i = 0; i < tot_proc_current; i++)
    if ((*current_procs)[i].pid == new_proc_pid)
      return i;

  return -1;
}

// armazena a quantidade maxima de PROCESSOS
// que podem ser armazenas na memoria da struct process_t
// antes que seja necessario realicar a memoria
uint32_t max_n_proc = 0;

int
get_process_active_con(process_t **cur_proc,
                       const int tot_cur_proc_act)
{
  int total_process = 0;
  unsigned int process_pids[MAX_PROCESS] = {0};

  total_process = get_numeric_directory(process_pids, MAX_PROCESS, PROCESS_DIR);

  if (total_process < 0)
    {
      perror("get_numeric_directory");
      exit(EXIT_FAILURE);
    }

  int total_conections = 0;
  conection_t conections[MAX_CONECTIONS] = {0};
  total_conections = get_info_conections(conections, MAX_CONECTIONS, PATH_INODE);

  if (total_conections < 0)
    {
      perror("get_info_conections");
      exit(EXIT_FAILURE);
    }

  if (debug)
    {
      printf("total de process %d\n", total_process);
      printf("tam struct process %zu\n", sizeof(process_t));
    }

  // process_t processos[MAX_PROCESS] = {0};
  //
  process_t processos[total_process];
  memset(processos, 0, total_process * sizeof(process_t));


  /*
   percorre todos os processos encontrados no diretório '/proc/',
   em cada processo encontrado armazena todos os file descriptors
   do processo - /proc/$id/fd - em process_t->fds,
   depois compara o link simbolico apontado pelo FD com 'socket:[inode]',
   sendo inode coletado do arquivo '/proc/net/tcp', caso a comparação seja igual,
   encontramos o processo que corresponde ao inode (conexão).
  */
  // file descriptors of process
  uint32_t fds_p[MAX_CONECTIONS] = {0}; // todos file descriptors de um processo
  char path_fd[MAX_PATH_FD] = {0};       // proc/pid/fd/
  char socket[MAX_NAME_SOCKET] = {0};   //socket:[99999999]
  char data_fd[MAX_NAME_SOCKET] = {0};  // dados lidos de um fd do processo

  int len_link = 0;

  int total_fd_process = 0;


  bool process_have_conection_active;
  uint32_t tot_process_active_con = 0; // contador de processos com conexões ativas

  uint32_t index_conection;
  int index_history_con[total_conections];
  // int index_history_con[MAX_CONECTIONS] = {0};

  for (int index_pd = 0; index_pd < total_process; index_pd++) // index_pd - process pid index
    {

      process_have_conection_active = false;

      index_conection = 0;
      memset(index_history_con, 0, total_conections * sizeof(int));


      snprintf(path_fd, MAX_PATH_FD, "/proc/%d/fd/", process_pids[index_pd]);

      // pegar todos os file descriptos do processo
      total_fd_process = get_numeric_directory(fds_p, MAX_CONECTIONS, path_fd);


      if (debug)
        printf("total_fd_process %d\n", total_fd_process);

      // falha ao abrir diretorio,
      // troca de processo
      if (total_fd_process < 0)
        continue;

      // passa por todos file descriptors do processo
      for (size_t id_fd = 0; id_fd < (uint32_t) total_fd_process; id_fd++)
        {
          // isola um fd por vez do processo
          snprintf(path_fd, MAX_PATH_FD, "/proc/%d/fd/%d", process_pids[index_pd], fds_p[id_fd]);

          if (debug)
            printf("path_fd %s\n", path_fd);
          // se der erro para ler, provavel acesso negado
          // vai pro proximo fd do processo
          if ((len_link = readlink(path_fd, data_fd, MAX_NAME_SOCKET)) == -1)
            continue;

          data_fd[len_link] = '\0';

          // caso o link nao tenha a palavra socket
          // vai para proximo fd do processo
          if (!strstr(data_fd, "socket"))
            continue;

          // compara o fd do processo com todos os inodes - conexões -  disponiveis
          for (int c = 0; c < total_conections; c++)
            {
              // connection in TIME_WAIT state, test next conection
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
        } // for fd


      if (process_have_conection_active)
        {

          // obtem informações do processo
          processos[tot_process_active_con].pid = process_pids[index_pd];
          processos[tot_process_active_con].total_fd = total_fd_process;
          processos[tot_process_active_con].total_conections = index_conection;

          int id_proc;
          id_proc = process_already_existed(cur_proc,
                                            tot_cur_proc_act,
                                            processos[tot_process_active_con].pid);
          if (id_proc != -1) // processo ja existe
            {
              // puts("processo ja existe");
              processos[tot_process_active_con].name = (*cur_proc)[id_proc].name;
              alloc_memory_conections(&processos[tot_process_active_con],
                                      &(*cur_proc)[id_proc]);
            }
          else // processo novo
            {
              // puts("allocando memoria para nome e conecxões");
              get_name_process(&processos[tot_process_active_con].name,
                               processos[tot_process_active_con].pid);

              alloc_memory_conections(&processos[tot_process_active_con], NULL);
            }


          for (uint32_t c = 0; c < processos[tot_process_active_con].total_conections; c++)
            {
              if (debug)
                printf("associando ao processo pid %d o inode %d\n",
                      processos[tot_process_active_con].pid, conections[index_history_con[c]].inode);
                // printf("index_histopry[%d] - %d\n", index_conection - c - 1, index_history_con[c])

              processos[tot_process_active_con].conection[c] = conections[index_history_con[c]];

              if (debug)
              printf("associado ao processo pid %d o inode %d - index %d\n",
                    processos[tot_process_active_con].pid, processos[tot_process_active_con].conection[c].inode, c);
            }

          // contabiliza total de processos que possuem conexao ativa
          tot_process_active_con++;
        }

    } //for process

  // sem processos com conexao ativa
  if (!tot_process_active_con){
    *cur_proc = NULL;
    return 0;
  }

  // alloca memoria para a struct passada por argumento
  // se chamada pela primeira vez - ponteiro == NULL - aloca
  // o dobro de memoria necessaria para um primeiro momento
  if ( ! *cur_proc )
    {
      *cur_proc = calloc( sizeof(process_t), (tot_process_active_con * 2) );

      if (! *cur_proc)
        {
          perror("calloc processos");
          exit(EXIT_FAILURE);
        }

      max_n_proc = tot_process_active_con * 2;
    }
  // se total de processos com conexões ativas for maior
  // que o espaço inicial reservado, realloca mais memoria
  else if ( tot_process_active_con > max_n_proc )
    {
      process_t *p;
      p = realloc(*cur_proc,
                  sizeof(process_t) * (tot_process_active_con * 2));

      if (! p)
        {
          perror("realloc processos");
          exit(EXIT_FAILURE);
        }

      *cur_proc = p;
      max_n_proc = tot_process_active_con * 2;
    }


  // free processes que nao tem mais conexoes ativas
  if (tot_cur_proc_act)
    free_process(*cur_proc, tot_cur_proc_act, processos, tot_process_active_con);

  // copia os processos com conexões ativos para
  // a struct process_t
  for (size_t i = 0; i < tot_process_active_con; i++)
    {
      if (*cur_proc)
        {
          for (int j = 0; j < tot_cur_proc_act; j++)
            {
              if ( (processos[i].pid == (*cur_proc)[j].pid) &&
                  ( (*cur_proc)[j].Bps_rx > 0 || (*cur_proc)[j].Bps_tx > 0 ) )
                {
                  processos[i].pps_rx = (*cur_proc)[j].pps_rx;
                  processos[i].pps_tx = (*cur_proc)[j].pps_tx;
                  processos[i].Bps_rx = (*cur_proc)[j].Bps_rx;
                  processos[i].Bps_tx = (*cur_proc)[j].Bps_tx;
                }
            }
        }
        (*cur_proc)[i] = processos[i];
    }



  // retorna o numero de processos com conexão ativa
  return tot_process_active_con;

}
