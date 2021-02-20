
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>    // variable errno
#include <stdbool.h>  // type boolean
#include <stdio.h>
#include <string.h>     // memset
#include <unistd.h>     // readliink, read
#include <sys/types.h>  // open
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/tcp.h>  // cods tcp state (TCP_ESTABLISHED, TCP_TIME_WAIT...)

#include "config.h"
#include "process.h"  // process_t
#include "m_error.h"  // ERROR_DEBUG

static int
get_name_process ( char **buffer, const pid_t pid );

static void
free_dead_process ( process_t *restrict cur_procs,
                    const size_t len_cur_procs,
                    const process_t *restrict new_procs,
                    const size_t len_new_procs );

// procura o pid no array de processos, caso encontra retorna o indice do array
static int
search_pid ( const pid_t search_pid,
             const process_t *procs,
             const size_t len_procs );

static void *
alloc_memory_conections ( process_t *new_st_processes,
                          const process_t *current_st_processes );

static void *
alloc_memory_process ( process_t **proc, const size_t len );

static void
process_copy ( process_t *restrict proc,
               process_t *restrict new_procs,
               const size_t new_tot_proc );

static void
save_statistics ( struct net_stat *restrict stat_dst,
                  struct net_stat *restrict stat_src );

static void
compile_conections ( conection_t *con,
                     conection_t *con_tot,
                     int *con_idx_proc,
                     const size_t len );

static void
copy_conections ( process_t *proc, conection_t *con, bool new_proc );

// armazena a quantidade maxima de PROCESSOS
// que podem ser armazenas na memoria da struct process_t
// principal antes que seja necessario realicar mais memoria
static uint32_t max_n_proc = 0;

/*
 percorre todos os processos encontrados no diretório '/proc/',
 em cada processo encontrado armazena todos os file descriptors
 do processo - /proc/$id/fd - no buffer fds_p
 depois compara o link simbolico apontado pelo FD com 'socket:[inode]',
 sendo inode coletado do arquivo '/proc/net/tcp', caso a comparação seja igual,
 encontramos o processo que corresponde ao inode (conexão).

 return total process with conection active or -1 on failure
*/
ssize_t
get_process_active_con ( process_t **cur_proc,
                         const size_t tot_cur_proc_act,
                         const struct config_op *co )
{
  // get pid all process in system
  uint32_t *process_pids;
  int total_process = get_numeric_directory2 ( &process_pids, PROCESS_DIR );
  if ( total_process == -1 )
    {
      ERROR_DEBUG ( "%s", "backtrace" );
      return -1;
    }

  // stores data only process with active conection
  process_t *processes = calloc ( total_process, sizeof ( *processes ) );
  if ( !processes )
    {
      ERROR_DEBUG ( "Alloc memory history pid: \"%s\"", strerror ( errno ) );
      return -1;
    }

  conection_t *conections;
  int total_conections = get_conections_system ( &conections, co->proto );

  if ( -1 == total_conections )
    {
      ERROR_DEBUG ( "%s", "backtrace" );
      return -1;
    }

  int *index_con_process =
          malloc ( total_conections * sizeof ( *index_con_process ) );
  if ( !index_con_process && total_conections > 0 )
    {
      ERROR_DEBUG ( "Alloc memory index: \"%s\"", strerror ( errno ) );
      return -1;
    }

  char path_fd[MAX_PATH_FD];      // proc/pid/fd/
  char socket[MAX_NAME_SOCKET];   // socket:[99999999]
  char data_fd[MAX_NAME_SOCKET];  // dados lidos de um fd do processo

  uint32_t *fds_p;  // todos file descriptors de um processo

  // all conections of one process (udp and tcp)
  conection_t *compiled_conections;

  // tamanho da string armazenada em data_fd por readlink
  ssize_t len_link;

  // total de file descriptors em /proc/<pid>/fd
  ssize_t total_fd_process;

  // true caso o processo tenha alguma conexão ativa
  // utilziada para associar a conexão ao processo
  bool process_have_conection_active;

  // true caso o processo tenha historico de rede (consultado no buffer
  // principal) com isso é mantido o historico
  bool process_have_conection_history;

  // contador de processos com conexões ativas
  size_t tot_process_active_con = 0;

  // armazena o total de conexões de um processo
  size_t tot_con_process;

  // new process?
  int exists_pid;

  // pass per each process in system
  // index_pd - process pid index
  for ( int index_pd = 0; index_pd < total_process; index_pd++ )
    {
      process_have_conection_history = false;
      process_have_conection_active = false;
      tot_con_process = 0;
      // fds_p = NULL;
      // compiled_conections = NULL;

      // verifica se o pid atual, consta na buffer principal
      exists_pid = search_pid (
              process_pids[index_pd], *cur_proc, tot_cur_proc_act );

      // se o pid constar no buffer principal e tiver historico de trafego de
      // de rede, essa indicação é habilitada para posteriormente salvar dados
      // das conexões
      if ( exists_pid != -1 &&
           ( ( *cur_proc )[exists_pid].net_stat.tot_Bps_rx ||
             ( *cur_proc )[exists_pid].net_stat.tot_Bps_tx ) )
        process_have_conection_history = true;

      snprintf ( path_fd, MAX_PATH_FD, "/proc/%d/fd/", process_pids[index_pd] );

      // pegar todos os file descriptos do processo
      total_fd_process = get_numeric_directory2 ( &fds_p, path_fd );

      // falha ao pegar file descriptos do processo,
      // troca de processo
      if ( total_fd_process == -1 )
        continue;

      // passa por todos file descriptors do processo
      for ( int id_fd = 0; id_fd < total_fd_process; id_fd++ )
        {
          // monta o path do file descriptor
          snprintf ( path_fd,
                     MAX_PATH_FD,
                     "/proc/%d/fd/%d",
                     process_pids[index_pd],
                     fds_p[id_fd] );

          // se der erro para ler
          // vai pro proximo fd do processo
          if ( ( len_link = readlink ( path_fd, data_fd, MAX_NAME_SOCKET ) ) ==
               -1 )
            continue;

          data_fd[len_link] = '\0';

          // caso o link nao comece com a palavra socket
          // vai para proximo fd do processo
          if ( strncmp ( data_fd, "socket", 6 ) )
            continue;

          // compara o fd do processo com todos os inodes - conexões -
          // disponiveis
          // test conections of process
          for ( int c = 0; c < total_conections; c++ )
            {
              // when a connection has the status TCP_TIME_WAIT the inode is 0
              // so there is no way easy to track the process for this
              // connection
              if ( conections[c].state == TCP_TIME_WAIT )
                continue;

              snprintf ( socket,
                         MAX_NAME_SOCKET,
                         "socket:[%d]",
                         conections[c].inode );

              // se o conteudo de socket - socket:[$inode] - for igual
              // ao valor lido do fd do processo,
              // encontramos de qual processo a conexão pertence
              if ( ( strncmp ( socket, data_fd, len_link ) ) == 0 )
                {
                  // salva o indice do array conections que tem a conexao
                  // do processo para depois pegar os dados desses indices
                  process_have_conection_active = true;
                  index_con_process[tot_con_process++] = c;
                }
            }

        }  // for id_fd

      free ( fds_p );

      if ( process_have_conection_active || process_have_conection_history )
        {
          // obtem informações do processo
          processes[tot_process_active_con].pid = process_pids[index_pd];
          processes[tot_process_active_con].total_conections = tot_con_process;

          // processo ja existe no buffer principal
          if ( exists_pid != -1 )
            {
              processes[tot_process_active_con].name =
                      ( *cur_proc )[exists_pid].name;

              // WARNING
              // passa a apontar para conexões do buffer principal
              if ( !alloc_memory_conections (
                           &processes[tot_process_active_con],
                           &( *cur_proc )[exists_pid] ) )
                return -1;

              // copia estatisticas globais atuais das conexões do processo
              // do buffer principal para o buffer temporario
              save_statistics ( &processes[tot_process_active_con].net_stat,
                                &( *cur_proc )[exists_pid].net_stat );
            }
          // processo novo
          else
            {
              if ( get_name_process ( &processes[tot_process_active_con].name,
                                      processes[tot_process_active_con].pid ) ==
                   -1 )
                return -1;

              if ( !alloc_memory_conections (
                           &processes[tot_process_active_con], NULL ) )
                return -1;
            }

          compiled_conections =
                  malloc ( processes[tot_process_active_con].total_conections *
                           sizeof ( *compiled_conections ) );
          if ( !compiled_conections )
            {
              ERROR_DEBUG ( "%s", strerror ( errno ) );
              return -1;
            }

          // pega todas as conexões e separa somente as que são referentes ao
          // processo
          compile_conections ( compiled_conections,
                               conections,
                               index_con_process,
                               tot_con_process );

          // copia as conexões referente ao processo,
          // se for um processo existente, copia apenas as conexões novas e
          // mantem as atuais ainda ativas com suas estatisticas, sem
          // altera-las. se for um processo novo copia todas
          copy_conections ( &processes[tot_process_active_con],
                            compiled_conections,
                            ( exists_pid == -1 ) );

          free ( compiled_conections );

          // contabiliza total de processos que possuem conexao ativa
          tot_process_active_con++;
        }

    }  // for process

  // se tem processos com conexão ativa
  if ( tot_process_active_con )
    {
      if ( !alloc_memory_process ( cur_proc, tot_process_active_con ) )
        return -1;

      // libera processos que nao estão no buffer principal
      free_dead_process (
              *cur_proc, tot_cur_proc_act, processes, tot_process_active_con );

      // copia os processes com conexões ativos para
      // o buffer principal struct process_t, mantendo as estatisticas de rede
      // dos processos que não são novos
      process_copy ( *cur_proc, processes, tot_process_active_con );
    }

  free ( process_pids );
  free ( processes );
  free ( conections );
  free ( index_con_process );

  // retorna o numero de processos com conexão ativa
  return tot_process_active_con;
}

// libera todos os processos
void
free_process ( process_t *proc, const size_t qtd_proc )
{
  if ( !proc )
    return;

  for ( size_t i = 0; i < qtd_proc; i++ )
    {
      free ( proc[i].name );
      free ( proc[i].conection );
    }

  free ( proc );
}

// compila todas as conexões que são referentes ao processo
static void
compile_conections ( conection_t *con_buff,
                     conection_t *con_tot,
                     int *con_idx_proc,
                     const size_t len )
{
  for ( size_t i = 0; i < len; i++ )
    con_buff[i] = con_tot[con_idx_proc[i]];
}

// deep copy struct net_stat
static void
save_statistics ( struct net_stat *restrict stat_dst,
                  struct net_stat *restrict stat_src )
{
  stat_dst->tot_Bps_rx = stat_src->tot_Bps_rx;
  stat_dst->tot_Bps_tx = stat_src->tot_Bps_tx;

  stat_dst->avg_Bps_rx = stat_src->avg_Bps_rx;
  stat_dst->avg_Bps_tx = stat_src->avg_Bps_tx;

  stat_dst->avg_pps_rx = stat_src->avg_pps_rx;
  stat_dst->avg_pps_tx = stat_src->avg_pps_tx;

  for ( size_t i = 0; i < LEN_BUF_CIRC_RATE; i++ )
    {
      stat_dst->Bps_rx[i] = stat_src->Bps_rx[i];
      stat_dst->Bps_tx[i] = stat_src->Bps_tx[i];
      stat_dst->pps_rx[i] = stat_src->pps_rx[i];
      stat_dst->pps_tx[i] = stat_src->pps_tx[i];
    }
}

// copia os processos com conexões ativos para
// o buffer principal
static void
process_copy ( process_t *restrict proc,
               process_t *restrict new_procs,
               size_t new_tot_proc )
{
  while ( new_tot_proc-- )
    *proc++ = *new_procs++;
}

// copia as conexões verificando se a conexão ja for existente
// mantem as statisticas de trafego de rede
// @proc      - processo que recebera/atualizara as conexões
// @con       - array com as conexões referentes ao processo
static void
copy_conections ( process_t *proc, conection_t *con, bool new_proc )
{
  int b = 0;
  bool skip;

  if ( new_proc )
    {
      for ( size_t c = 0; c < proc->total_conections; c++ )
        proc->conection[c] = con[c];

      return;
    }

  // copia apenas as conexões novas para o processo
  // as conexões que ja estão no processo, não são tocadas
  for ( size_t c = 0; c < proc->total_conections; c++ )
    {
      skip = false;

      // loop "reverso" interno parece ter um desempenho melhor
      for ( int a = ( int ) proc->total_conections - 1; a >= b; a-- )
        {
          // se a conexão ja estiver mapeada no processo, pula ela
          // para não perder as estatisticas dela
          if ( proc->conection[c].inode == con[a].inode )
            {
              skip = true;

              // troca o conteudo da posição atual pelo conteudo do ultima da
              // fila, assim podemos diminuir o laço interno
              if ( a != b )
                con[a] = con[b];

              // diminiu o tamanho do laço interno (para performance)
              b++;

              break;
            }
        }
      // conexão ja esta no buffer principal com suas estatisticas,
      // não precisa mecher
      if ( skip )
        continue;

      // conexão nova, logo não tem estatisticas a serem copiadas, copia apenas
      // os dados de identificação conexão
      proc->conection[c] = con[c];
    }
}

// alloca memoria para process_t com o dobro do tamanho informado
// se chamada pela primeira vez - ponteiro == NULL,
// se não, verifica se o espaço de memoria atual é
// insuficiente com base no numero de processos ativos x alocação anterior
static void *
alloc_memory_process ( process_t **proc, const size_t len )
{
  const size_t new_len = len * 2;

  // na primeira vez sera nulo, aloca o dobro da quantidade necessaria
  if ( !*proc )
    {
      *proc = calloc ( sizeof ( process_t ), new_len );

      if ( !*proc )
        {
          ERROR_DEBUG ( "%s", strerror ( errno ) );
          return NULL;
        }

      max_n_proc = new_len;
    }
  // se total de processos com conexões ativas agora for maior
  // que o espaço inicial reservado, realloca mais memoria (o dobro necessario).
  // OU
  // se o espaço alocado para os processos estiver três vezes maior que o
  // necessario no momento, diminui 1/3 do espaço alocado, ainda mantendo o
  // dobro do necessario, assim temos a oportunidade de economizar memória e
  // tambem evitar muitos reallocs
  else if ( len > max_n_proc || max_n_proc >= len * 3 )
    {
      void *p;
      p = realloc ( *proc, sizeof ( process_t ) * new_len );

      if ( !p )
        {
          ERROR_DEBUG ( "%s", strerror ( errno ) );
          return NULL;
        }

      *proc = p;
      max_n_proc = new_len;
    }

  return *proc;
}

// aloca memoria INICIAL para conection da estrutura process_t
// com base no numero de conexoes que o processo tem,
// sendo numero de conexoes * 2 o tamanho alocado.
// Antes de reallocar mais memoria é feita verificações para checar se o valor
// alocado inicialmente não atende, caso não realocamos para nova quantidade
// de conexões * 2;
static void *
alloc_memory_conections ( process_t *new_st_processes,
                          const process_t *current_st_processes )
{
  // total de memória alocada para as conexões é no minimo de uma posição
  const size_t new_len = ( new_st_processes->total_conections )
                                 ? new_st_processes->total_conections * 2
                                 : 1;

  // processo novo, aloca duas vezes quantidade de memoria necessaria
  // para evitar realocar com frequencia...
  if ( !current_st_processes )
    {
      new_st_processes->conection = calloc ( sizeof ( conection_t ), new_len );

      if ( !new_st_processes->conection )
        {
          ERROR_DEBUG ( "%s", strerror ( errno ) );
          return NULL;
        }

      new_st_processes->max_n_con = new_len;
    }
  // status atual do processo nao tem memoria suficiente
  // para armazenas a quantidade de conexões do novo status processo
  // realoca memoria para o dobro da nova demanda.
  // OU
  // se o espaço alocado for três vezes maior que a nova demanda, diminiu
  // o espaço alocado
  else if ( current_st_processes->max_n_con <
                    new_st_processes->total_conections ||
            current_st_processes->max_n_con >= new_len * 3 )
    {
      conection_t *p = NULL;
      p = realloc ( current_st_processes->conection,
                    new_len * sizeof ( conection_t ) );

      if ( !p )
        {
          ERROR_DEBUG ( "%s", strerror ( errno ) );
          return NULL;
        }

      // memória alocado foi aumentada, inicializa apenas o espaço novo
      if ( new_st_processes->total_conections >
           current_st_processes->max_n_con )
        {
          memset ( p + current_st_processes->max_n_con,
                   0,
                   ( new_len - current_st_processes->max_n_con ) *
                           sizeof ( conection_t ) );
        }

      new_st_processes->conection = p;
      new_st_processes->max_n_con = new_len;
    }
  // apenas reutiliza a memoria ja alocada, espaço atual esta ok...
  else
    {
      new_st_processes->conection = current_st_processes->conection;
      new_st_processes->max_n_con = current_st_processes->max_n_con;
    }

  return new_st_processes->conection;
}

// verifica se o pid ja existe no buffer.
// retorna o indice do buffer em que o pid foi encontrado
// ou -1 caso o pid não seja localizado
// @param pid_t pid, o pid a ser buscado
// @param ponteiro process_t procs, o buffer a procurar
// @param size_t len_procs, tamanho do buffer procs
static int
search_pid ( const pid_t pid, const process_t *procs, size_t len_procs )
{
  if ( !procs )
    return -1;

  for ( size_t i = 0; i < len_procs; i++ )
    if ( procs[i].pid == pid )
      return i;

  return -1;
}

// libera processos correntes que não foram localizados na mais nova checagem
// por processos com conexões ativas, ou seja, processos que encerram e/ou não
// possuem conexão ativa no momento.
// obs: comexão ativa = conexão listada no arquivo /proc/net/tcp | udp
static void
free_dead_process ( process_t *restrict cur_procs,
                    const size_t len_cur_procs,
                    const process_t *restrict new_procs,
                    const size_t len_new_procs )
{
  for ( size_t i = 0; i < len_cur_procs; i++ )
    {
      // se processo não localizado
      // liberando memoria alocada para seus atributos
      if ( -1 == ( search_pid ( cur_procs[i].pid, new_procs, len_new_procs ) ) )
        {
          free ( cur_procs[i].name );
          free ( cur_procs[i].conection );
        }
    }
}

static ssize_t
full_read ( const int fd, void *buffer, size_t count )
{
  ssize_t bytes_read;
  ssize_t total_read = 0;

  while ( count > 0 )
    {
      bytes_read = read ( fd, buffer, count );
      if ( bytes_read == -1 )
        {
          if ( errno == EINTR )
            continue;
          else
            {
              ERROR_DEBUG ( "%s", strerror ( errno ) );
              return -1;
            }
        }
      else if ( bytes_read )
        {
          buffer = ( char * ) buffer + bytes_read;
          count -= bytes_read;
          total_read += bytes_read;
        }
      else
        break;
    }

  return total_read;
}

// armazena o nome do processo no buffer e retorna
// o tamanho do nome do processo,
// função cuida da alocação de memoria para o nome do processo
static int
get_name_process ( char **buffer, const pid_t pid )
{
  char path_cmdline[MAX_CMDLINE];
  char *temp;
  ssize_t total_read;
  int fd;

  snprintf ( path_cmdline, MAX_CMDLINE, "/proc/%d/cmdline", pid );

  if ( -1 == ( fd = open ( path_cmdline, O_RDONLY ) ) )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return -1;
    }

  // FIXME: that's enough?
  if ( !( *buffer = malloc ( 4096 ) ) )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return -1;
    }

  if ( -1 == ( total_read = full_read ( fd, *buffer, 4096 ) ) )
    {
      free ( *buffer );
      return -1;
    }

  close ( fd );

  // decreases excess memory
  if ( NULL != ( temp = realloc ( *buffer, total_read ) ) )
    *buffer = temp;

  // warranty in case realloc fail
  temp = *buffer;

  // run (total_read -1) times
  while ( --total_read )
    {
      if ( *temp == '\0' || *temp == '\n' )
        *temp = ' ';

      temp++;
    }

  *temp = '\0';

  return 1;
}
