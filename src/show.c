
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

// #include <stdio.h>
#include <stdbool.h>
#include <string.h>  // strlen
// #include <term.h>    // variable columns
#include <ncurses.h>

#include "process.h"
#include "conection.h"
#include "terminal.h"  // clear_cmd
#include "translate.h"

// defined in main.c
extern bool view_conections;

// tamanho representação textual porta da camada de transporte
#define PORTLEN 6  // 65535 + '\0'

// "ddd.ddd.ddd.ddd:ppppp <-> ddd.ddd.ddd.ddd:ppppp"
#define LEN_TUPLE ( ( INET_ADDRSTRLEN + PORTLEN ) * 2 ) + 7

// espaçamento entre as colunas
#define PID -5  // negativo alinhado a esquerda
#define PPS 6
#define RATE 13
#define PROGRAM 49  // tamanho fixo de caracteres até a coluna program

// total de espaço até inicio da coluna
// #define START_COL_PROG 49

// maximo de caracteres que sera exibido no nome de um processo
// #define LEN_NAME_PROGRAM 44

static void
print_conections ( const process_t *process );

int
find_last_char(const char *str, const char ch, size_t len);

void
show_process ( const process_t *const processes, const size_t tot_process )
{
  int y, x;
  int len_prog, len_last;

  WINDOW *my_pad;

  my_pad = newpad(120,120);


  // limpa a tela e o scrollback
  clear_cmd ();

  // attron ( A_REVERSE | COLOR_PAIR ( 2 ) );
  printw (" %*s %*s %*s     %s       %s   %s\n",
           PID,
           "PID",
           PPS,
           "PPS TX",
           PPS,
           "PPS RX",
           "RATE TX",
           "RATE RX",
           "PROGRAM" );
  // attroff ( A_REVERSE | COLOR_PAIR ( 2 ) );
  getyx(stdscr, y, x);
  mvchgat(0, 0, -1, A_REVERSE, 2, NULL);
  move(y, x);

  attron ( A_BOLD );
  for ( size_t i = 0; i < tot_process; i++ )
    {
      // só exibe o processo se tiver fluxo de rede
      if ( processes[i].net_stat.avg_Bps_rx ||
           processes[i].net_stat.avg_Bps_tx ||
           processes[i].net_stat.avg_pps_rx ||
           processes[i].net_stat.avg_pps_tx )
        {

          printw ( " %*d %*ld %*ld ",
                   PID,
                   processes[i].pid,
                   PPS,
                   processes[i].net_stat.avg_pps_tx,
                   PPS,
                   processes[i].net_stat.avg_pps_rx );

          printw ( "%*s %*s ",
                   RATE,
                   // "1023.69 kib/s",
                   processes[i].net_stat.tx_rate,
                   RATE,
                   // "1023.69 kib/s");
                   processes[i].net_stat.rx_rate );

          // attron (COLOR_PAIR(1));
          printw ( "%*.*s\n", -(COLS - PROGRAM -1), COLS - PROGRAM - 1, processes[i].name );
          // attroff (COLOR_PAIR(1));
          len_prog = strlen_space(processes[i].name);
          len_last = find_last_char(processes[i].name, '/', len_prog );

          getyx(stdscr, y, x);
          mvchgat(y - 1, PROGRAM, -1, A_NORMAL, 1, NULL); // pinta nome do programa até a ultima '/'
          mvchgat(y - 1, PROGRAM + len_last + 1, len_prog - len_last - 1, A_BOLD, 1, NULL); // pinta o nome final do programa
          move(y, x);

          if ( view_conections )
            print_conections ( &processes[i] );
        }

    }
  attroff ( A_BOLD );
  prefresh (my_pad, 0,0,0,0,20,30);
  refresh ();
}

static void
print_conections ( const process_t *process )
{
  // tuple ip:port <-> ip:port
  char *tuple;

  size_t index_act[process->total_conections];
  size_t tot_con_act;
  // pega o total de conexões no processo que estão ativas, e seus respectivos
  // indices
  tot_con_act = get_con_active_in_process (
          index_act, process->conection, process->total_conections );

  attron ( A_DIM );
  for ( size_t i = 0; i < tot_con_act; i++ )
    {
      // faz a tradução de ip:porta para nome:serviço
      tuple = translate ( &process->conection[index_act[i]] );

      printw ( " %*s %*ld %*ld %*s %*s ",
               PID,
               "",
               PPS,
               process->conection[index_act[i]].net_stat.avg_pps_tx,
               PPS,
               process->conection[index_act[i]].net_stat.avg_pps_rx,
               RATE,
               process->conection[index_act[i]].net_stat.tx_rate,
               RATE,
               process->conection[index_act[i]].net_stat.rx_rate );

      attron ( COLOR_PAIR ( 1 ) );
      if ( i + 1 != tot_con_act )
        {
          addch ( ACS_LTEE );   // ├
          addch ( ACS_HLINE );  // ─
        }
      else
        {                          // ultima conexão
          addch ( ACS_LLCORNER );  // └
          addch ( ACS_HLINE );     // ─
        }
      attroff ( COLOR_PAIR ( 1 ) );

      printw ( " %s\n", tuple );
    }
  addch ( '\n' );
  attroff (A_DIM );
}

// retorna a posição do ultimo caracter pesquisado
int
find_last_char(const char *str, const char ch, size_t len)
{
  size_t i = (len) ? len : strlen_space(str);

  while(i)
    {
      if (str[i] == ch)
        return i;

      i--;
    }

  // not found
  return -1;
}
