
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

#include <errno.h>    // variable errno
#include <stdbool.h>  // type boolean
#include <stdio.h>
#include <string.h>  // memset
#include <unistd.h>  // readliink

#include "config.h"
#include "process.h"  // process_t
#include "m_error.h"  // fatal_error, error

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

static void
alloc_memory_conections ( process_t *new_st_processes,
                          const process_t *current_st_processes );

static void
alloc_memory_process ( process_t **proc, const size_t len );

static void
process_copy ( process_t *restrict proc,
               process_t *restrict new_procs,
               const size_t new_tot_proc );

static void
save_statistics ( struct net_stat *restrict stat_dst,
                  struct net_stat *restrict stat_src );

static void
copy_conections ( process_t *proc,
                  conection_t *con,
                  int *index_con,
                  const size_t tot_con );

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
*/
int
get_process_active_con ( process_t **cur_proc,
                         const size_t tot_cur_proc_act,
                         const struct config_op *co )
{
  // buffer utilizado na função get_process_active_con
  // para armazenar o PID de todos os processos do sistema
  static uint32_t process_pids[MAX_PROCESS] = {0};

  // armazena os dados apenas dos processos que possuem conexão ativa
  // FIXME: verificar migração para memoria dinamica para o buffer "processos"
  static process_t processos[MAX_PROCESS] = {0};

  // buffer utilziado na função get_process_active_con
  // para armazenas todas as conexões do sistema
  // static conection_t conections[MAX_CONECTIONS] = {0};
  conection_t *conections;

  // buffer utilizado para armazenar conexões individuais dos processos
  // static int index_history_con[MAX_CONECTIONS] = {0}
  // int *index_history_con;

  int total_process =
          get_numeric_directory ( process_pids, MAX_PROCESS, PROCESS_DIR );

  if ( total_process == -1 )
    fatal_error ( "Error get PIDs of processes" );

  // int total_conections = get_info_conections (
  //         conections, MAX_CONECTIONS, ( co->udp ) ? PATH_UDP : PATH_TCP );

  int total_conections;
  total_conections = get_info_conections2 ( &conections,
                                            ( co->udp ) ? PATH_UDP : PATH_TCP );

  if ( total_conections == -1 )
    fatal_error ( "Error get_info_conections" );

  int *index_history_con;
  index_history_con = calloc ( total_conections, sizeof ( int ) );

  // todos file descriptors de um processo
  uint32_t fds_p[MAX_CONECTIONS] = {0};
  char path_fd[MAX_PATH_FD] = {0};      // proc/pid/fd/
  char socket[MAX_NAME_SOCKET] = {0};   // socket:[99999999]
  char data_fd[MAX_NAME_SOCKET] = {0};  // dados lidos de um fd do processo

  // tamanho da string armazenada em data_fd por readlink
  int len_link;

  // total de file descriptors em /proc/<pid>/fd
  int total_fd_process;

  // true caso o processo tenha alguma conexão ativa
  // utilziada para associar a conexão ao processo
  bool process_have_conection_active;

  // true caso o processo tenha history de rede (consultado no buffer principal)
  // com isso é mantido o historico
  bool process_have_conection_history;

  // contador de processos com conexões ativas
  uint32_t tot_process_active_con = 0;

  // armazena o indice que contem os dados referente a conexão
  // do processo no buffer conections
  uint32_t index_conection;
  // int index_history_con[total_conections];
  int exists_pid;

  for ( int index_pd = 0; index_pd < total_process;
        index_pd++ )  // index_pd - process pid index
    {
      // verifica se o pid atual, consta na buffer principal
      exists_pid = search_pid (
              process_pids[index_pd], *cur_proc, tot_cur_proc_act );

      process_have_conection_history = false;
      process_have_conection_active = false;

      // se o pid constar no buffer principal e tiver historico de trafego de
      // de rede, essa indicação é habilitada
      if ( exists_pid != -1 )
        if ( ( *cur_proc )[exists_pid].net_stat.tot_Bps_rx ||
             ( *cur_proc )[exists_pid].net_stat.tot_Bps_tx )
          process_have_conection_history = true;

      index_conection = 0;
      // memset ( index_history_con, 0, MAX_CONECTIONS * sizeof ( int ) );

      snprintf ( path_fd, MAX_PATH_FD, "/proc/%d/fd/", process_pids[index_pd] );

      // pegar todos os file descriptos do processo
      total_fd_process =
              get_numeric_directory ( fds_p, MAX_CONECTIONS, path_fd );

      // falha ao pegar file descriptos do processo,
      // troca de processo
      if ( total_fd_process <= 0 )
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

          // caso o link nao tenha a palavra socket
          // vai para proximo fd do processo
          if ( !strstr ( data_fd, "socket" ) )
            continue;

          // compara o fd do processo com todos os inodes - conexões -
          // disponiveis
          for ( int c = 0; c < total_conections; c++ )
            {
              // connection in TIME_WAIT state, test next conection
              if ( conections[c].inode == 0 )
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
                  index_history_con[index_conection++] = c;
                }
            }
        }  // for id_fd

      if ( process_have_conection_active || process_have_conection_history )
        {
          // obtem informações do processo
          processos[tot_process_active_con].pid = process_pids[index_pd];
          processos[tot_process_active_con].total_fd = total_fd_process;
          processos[tot_process_active_con].total_conections = index_conection;

          // processo ja existe no buffer principal
          if ( exists_pid != -1 )
            {
              processos[tot_process_active_con].name =
                      ( *cur_proc )[exists_pid].name;
              alloc_memory_conections ( &processos[tot_process_active_con],
                                        &( *cur_proc )[exists_pid] );

              save_statistics ( &processos[tot_process_active_con].net_stat,
                                &( *cur_proc )[exists_pid].net_stat );
            }
          else  // processo novo
            {
              get_name_process ( &processos[tot_process_active_con].name,
                                 processos[tot_process_active_con].pid );

              alloc_memory_conections ( &processos[tot_process_active_con],
                                        NULL );
              // zera estrutura net_stat, para evitar sujeira de processo antigo
              // FIXME: verificar migração para memoria dinamica para o buffer
              // "processos"
              memset ( &processos[tot_process_active_con].net_stat,
                       0,
                       sizeof ( struct net_stat ) );
            }

          // copia as conexões verificando se a conexão ja for existente
          // mantem as statisticas de trafego de rede
          copy_conections (
                  &processos[tot_process_active_con],
                  conections,
                  index_history_con,
                  processos[tot_process_active_con].total_conections );

          // contabiliza total de processos que possuem conexao ativa
          tot_process_active_con++;
        }

    }  // for process

  // FIXME: checar o resultado dessa decisão
  // sem processos com conexao ativa
  if ( tot_process_active_con == 0 )
    {
      *cur_proc = NULL;
      return 0;
    }

  alloc_memory_process ( cur_proc, tot_process_active_con );

  // libera processos que nao estão no buffer principal
  if ( tot_cur_proc_act )
    free_dead_process (
            *cur_proc, tot_cur_proc_act, processos, tot_process_active_con );

  // copia os processos com conexões ativos para
  // o buffer principal struct process_t, mantendo as estatisticas de rede
  // dos processos que não são novos
  process_copy ( *cur_proc, processos, tot_process_active_con );

  free ( conections );
  free ( index_history_con );

  // retorna o numero de processos com conexão ativa
  return tot_process_active_con;
}

// libera todos os processos
void
free_process ( process_t *proc, const size_t qtd_proc )
{
  for ( size_t i = 0; i < qtd_proc; i++ )
    {
      free ( proc[i].name );
      free ( proc[i].conection );
    }

  free ( proc );
}

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
// o buffer principal struct process_t, mantendo as estatisticas
// dos processos que não são novos e possem
static void
process_copy ( process_t *restrict proc,
               process_t *restrict new_procs,
               const size_t new_tot_proc )
{
  // memset(proc, 0, new_tot_proc * sizeof(process_t));
  for ( size_t i = 0; i < new_tot_proc; i++ )
    {
      // copia conteudo do buffer temporario para buffer principal
      *( proc + i ) = *( new_procs + i );
    }
}

// copia as conexões verificando se a conexão ja for existente
// mantem as statisticas de trafego de rede
// @proc      - processo que recebera/atualizara as conexões
// @con       - array com as conexões ativas no sistema
// @index_con - array que identifica somente as conexões do processo em questão
// @tot_con   - total de conexões que o processo possui
static void
copy_conections ( process_t *proc,
                  conection_t *con,
                  int *index_con,
                  const size_t tot_con )
{
  // conection_t temp;
  int b = 0;
  bool skip;

  // copia apenas as conexões novas para o processo
  // as conexões que ja estão no processo, não são tocadas
  for ( size_t c = 0; c < tot_con; c++ )
    {
      skip = false;

      // loop "reverso" interno parece ter um desempenho melhor
      for ( int a = ( int ) tot_con - 1; a >= b; a-- )
        {
          // se a conexão ja estiver mapeada no processo, pula ela
          if ( proc->conection[c].inode == con[index_con[a]].inode )
            {
              skip = true;

              // se não for a primeira conexão do array interno, troca o
              // conteudo da posição atual pelo conteudo da primeira,
              // assim podemos diminuir o laço interno
              if ( a != b )
                {
                  con[index_con[a]] = con[index_con[b]];
                  // con[index_con[a]].if_index = con[index_con[b]].if_index;
                  // con[index_con[a]].inode = con[index_con[b]].inode;
                  // con[index_con[a]].local_address =
                  // con[index_con[b]].local_address;
                  // con[index_con[a]].remote_address =
                  // con[index_con[b]].remote_address;
                  // con[index_con[a]].local_port =
                  // con[index_con[b]].local_port; con[index_con[a]].remote_port
                  // = con[index_con[b]].remote_port;
                  save_statistics ( &con[index_con[a]].net_stat,
                                    &con[index_con[b]].net_stat );
                }

              // diminiu o tamanho do laço interno (para performance)
              b++;

              break;
            }
        }
      // conexão ja esta no buffer com suas estatisticas, não precisa mecher
      if ( skip )
        continue;

      proc->conection[c] = con[index_con[c]];
      // proc->conection[c].if_index = con[index_con[b]].if_index;
      // proc->conection[c].inode = con[index_con[b]].inode;
      // proc->conection[c].local_address = con[index_con[b]].local_address;
      // proc->conection[c].remote_address = con[index_con[b]].remote_address;
      // proc->conection[c].local_port = con[index_con[b]].local_port;
      // proc->conection[c].remote_port = con[index_con[b]].remote_port;
      save_statistics ( &proc->conection[c].net_stat,
                        &con[index_con[b]].net_stat );
    }
}

// alloca memoria para process_t com o dobro do tamanho informado
// se chamada pela primeira vez - ponteiro == NULL,
// se não, verifica se o espaço de memoria atual é
// insuficiente com base no numero de processos ativos x alocação anterior
static void
alloc_memory_process ( process_t **proc, const size_t len )
{
  // na primeira vez sera nulo, aloca o dobro da quantidade necessaria
  if ( !*proc )
    {
      *proc = calloc ( sizeof ( process_t ), ( len * 2 ) );

      if ( !*proc )
        fatal_error ( "Alloc memory process, %s", strerror ( errno ) );

      max_n_proc = len * 2;
    }
  // se total de processos com conexões ativas for maior
  // que o espaço inicial reservado, realloca mais memoria (o dobro necessario)
  else if ( len > max_n_proc )
    {
      void *p;
      p = realloc ( *proc, sizeof ( process_t ) * len * 2 );

      if ( !p )
        fatal_error ( "Realloc memory process, %s", strerror ( errno ) );

      *proc = p;
      max_n_proc = len * 2;
    }
}

// aloca memoria INICIAL para conection da estrutura process_t
// com base no numero de conexoes que o processo tem,
// sendo numero de conexoes * 2 o tamanho alocado.
// Antes de reallocar mais memoria é feita verificações para checar se o valor
// alocado inicialmente não atende, caso não realocamos para nova quantidade
// de conexões * 2;
static void
alloc_memory_conections ( process_t *new_st_processes,
                          const process_t *current_st_processes )
{
  // processo novo, aloca duas vezes quantidade de memoria necessaria
  // para evitar realocar com frequencia...
  if ( !current_st_processes )
    {
      new_st_processes->conection = calloc (
              sizeof ( conection_t ), new_st_processes->total_conections * 2 );

      if ( !new_st_processes->conection )
        fatal_error ( "Alloc conection memory new process, %s",
                      strerror ( errno ) );

      new_st_processes->max_n_con = new_st_processes->total_conections * 2;
    }
  // status atual do processo nao tem memoria suficiente
  // para armazenas a quantidade de conexões do novo status processo
  // realoca memoria para o dobro da nova demanda.
  else if ( current_st_processes->max_n_con <
            new_st_processes->total_conections )
    {
      void *p = NULL;
      p = realloc ( current_st_processes->conection,
                    new_st_processes->total_conections *
                            sizeof ( conection_t ) * 2 );

      if ( !p )
        fatal_error ( "Realloc conection memory process, %s",
                      strerror ( errno ) );

      new_st_processes->conection = p;
      new_st_processes->max_n_con = new_st_processes->total_conections * 2;
    }
  // apenas reutiliza a memoria ja alocada, espaço é suficiente...
  else
    {
      new_st_processes->conection = current_st_processes->conection;
      new_st_processes->max_n_con = current_st_processes->max_n_con;
    }
}

// verifica se o pid ja existe no buffer.
// retorna o indice do buffer em que o pid foi encontrado
// ou -1 caso o pid não seja localizado
// @param pid_t search_pid, o pid a ser buscado
// @param ponteiro process_t procs, o buffer a procurar
// @param size_t len_procs, tamanho do buffer procs
static int
search_pid ( const pid_t search_pid,
             const process_t *procs,
             const size_t len_procs )
{
  if ( !procs )
    return -1;

  for ( size_t i = 0; i < len_procs; i++ )
    if ( procs[i].pid == search_pid )
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
      // locate = false;
      int id = -1;
      id = search_pid ( cur_procs[i].pid, new_procs, len_new_procs );

      // processo não localizado
      // liberando memoria alocada para seus atributos
      if ( id == -1 )
        {
          free ( cur_procs[i].name );
          free ( cur_procs[i].conection );
        }
    }
}

// armazena o nome do processo no buffer e retorna
// o tamanho do nome do processo incluindo null bytes ou espaço,
// função cuida da alocação de memoria para o nome do processo
static int
get_name_process ( char **buffer, const pid_t pid )
{
  char path_cmdline[MAX_NAME];
  snprintf ( path_cmdline, MAX_NAME, "/proc/%d/cmdline", pid );

  FILE *arq = NULL;
  arq = fopen ( path_cmdline, "r" );

  if ( arq == NULL )
    {
      error ( "Open file, %s", strerror ( errno ) );
      return -1;
    }

  char line[MAX_NAME];
  if ( !fgets ( line, MAX_NAME, arq ) )
    {
      fclose ( arq );
      error ( "Read file, %s", strerror ( errno ) );
      return -1;
    }

  fclose ( arq );

  // tamanho até null byte ou primeiro espaço
  // size_t len = strlen_space ( line );
  size_t len = strlen ( line );

  // fgets ja coloca null byte
  // line[len] = '\0';

  *buffer = calloc ( 1, len + 1 );

  if ( !*buffer )
    {
      error ( "Alloc buffer name, %s", strerror ( errno ) );
      return -1;
    }

  // copia a string junto com null byte
  size_t i;
  for ( i = 0; i < len + 1; i++ )
    ( *buffer )[i] = line[i];

  return i;
}
