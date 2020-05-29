
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

#include "str.h"
#include "process.h"
#include "conection.h"
#include "terminal.h"  // clear_cmd
#include "translate.h"
#include "show.h"

// defined in main.c
extern bool view_conections;
extern process_t *processes;
extern uint32_t tot_process_act;

extern WINDOW *pad;

// tamanho representação textual porta da camada de transporte
#define PORTLEN 5  // 65535

// "ddd.ddd.ddd.ddd:ppppp <-> ddd.ddd.ddd.ddd:ppppp"
#define LEN_TUPLE ( ( INET_ADDRSTRLEN + PORTLEN ) * 2 ) + 7

// espaçamento entre as colunas
#define PID -5  // negativo alinhado a esquerda
#define PPS 6
#define RATE 13

// // tamanho fixo de caracteres até a coluna program
// #define PROGRAM 49

static int scroll_x = 0;
static int scroll_y = 1;
static int selected = 1;  // posição de linha do item selecionado
static int tot_rows;      // total linhas exibidas

static void
print_conections ( const process_t *process );

void
show_process ( const process_t *const processes, const size_t tot_process )
{
  int y, x;
  int base_name, name;

  // limpa a tela e o scrollback
  wclear ( pad );

  wprintw ( pad,
            " %*s %*s %*s     %s       %s   %s\n",
            PID,
            "PID",
            PPS,
            "PPS TX",
            PPS,
            "PPS RX",
            "RATE TX",
            "RATE RX",
            "PROGRAM" );
  // paint line header
  getyx ( pad, y, x );
  mvwchgat ( pad, 0, 0, -1, A_REVERSE, 2, NULL );
  wmove ( pad, y, x );

  tot_rows = 0;

  wattron ( pad, A_BOLD );
  for ( size_t i = 0; i < tot_process; i++ )
    {
      // só exibe o processo se tiver fluxo de rede
      if ( processes[i].net_stat.avg_Bps_rx ||
           processes[i].net_stat.avg_Bps_tx ||
           processes[i].net_stat.avg_pps_rx ||
           processes[i].net_stat.avg_pps_tx )
        {
          tot_rows++;
          wprintw ( pad,
                    " %*d %*ld %*ld ",
                    PID,
                    processes[i].pid,
                    PPS,
                    processes[i].net_stat.avg_pps_tx,
                    PPS,
                    processes[i].net_stat.avg_pps_rx );

          wprintw ( pad,
                    "%*s %*s ",
                    RATE,
                    // "1023.69 kib/s",
                    processes[i].net_stat.tx_rate,
                    RATE,
                    // "1023.69 kib/s");
                    processes[i].net_stat.rx_rate );

          wprintw ( pad, "%s\n", processes[i].name );

          base_name = strlen_space ( processes[i].name );
          name = find_last_char ( processes[i].name, base_name, '/' ) + 1;

          getyx ( pad, y, x );
          mvwchgat ( pad,
                     y - 1,
                     PROGRAM,
                     -1,
                     A_NORMAL,
                     1,
                     NULL );  // pinta linha inteira do nome do programa
          mvwchgat ( pad,
                     y - 1,
                     PROGRAM + name ,
                     base_name - name,
                     A_BOLD,
                     1,
                     NULL );  // pinta somente o nome do programa
          wmove ( pad, y, x );

          if ( view_conections )
            print_conections ( &processes[i] );
        }

    }
  wattroff ( pad, A_BOLD );

  // paint item selected
  if (tot_rows)
    {
      if (selected > tot_rows)
        selected = tot_rows;

      getyx ( pad, y, x );
      mvwchgat(pad, selected, 0, -1, A_REVERSE, 1, NULL);
      wmove ( pad, y, x );
    }

  prefresh ( pad, 0, scroll_x, 0, 0, 0, COLS - 1 ); // atualiza cabeçalho
  prefresh ( pad, scroll_y, scroll_x, 1, 0, LINES - 1, COLS - 1 );
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

  wattron ( pad, A_DIM );
  for ( size_t i = 0; i < tot_con_act; i++ )
    {
      tot_rows++;

      // faz a tradução de ip:porta para nome:serviço
      tuple = translate ( &process->conection[index_act[i]] );

      wprintw ( pad,
                " %*s %*ld %*ld %*s %*s ",
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

      wattron ( pad, COLOR_PAIR ( 1 ) );
      if ( i + 1 != tot_con_act )
        {
          waddch ( pad, ACS_LTEE );   // ├
          waddch ( pad, ACS_HLINE );  // ─
        }
      else
        {                                // ultima conexão
          waddch ( pad, ACS_LLCORNER );  // └
          waddch ( pad, ACS_HLINE );     // ─
        }
      wattroff ( pad, COLOR_PAIR ( 1 ) );

      wprintw ( pad, " %s\n", tuple );
    }
  waddch ( pad, '\n' );
  wattroff ( pad, A_DIM );
}



void
ui_tick ()
{
  int ch;
  while ( (ch = wgetch(pad)) != ERR )
    {
      switch ( ch )
        {
          // scroll horizontal
          case KEY_RIGHT:
            if (scroll_x < COLS_PAD )
              {
                scroll_x += 5;
                prefresh ( pad, 0, scroll_x, 0, 0, LINES - 1, COLS - 1 );
              }
            else
              beep();

            break;
          case KEY_LEFT:
            if (scroll_x > 0)
              {
                scroll_x -= 5;
                prefresh ( pad, 0, scroll_x, 0, 0, LINES - 1, COLS - 1 );
              }
            else
              beep();

            break;
          case KEY_DOWN:
            if (selected + 1  <= tot_rows)
              {
                selected++;
                if (selected >= LINES)
                  scroll_y++;

                show_process ( processes, tot_process_act );
              }
            else
              beep();


            break;
          case KEY_UP:
            if (selected - 1  >= 1)
              {
                selected--;
                if ( scroll_y > 1 && selected <= LINES)
                  scroll_y--;

                show_process ( processes, tot_process_act );
              }
            else
              beep();

            break;
        }
    }
}
