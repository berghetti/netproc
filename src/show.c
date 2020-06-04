
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

// #include <stdlib.h>
#include <stdbool.h>
#include <string.h>  // strlen
#include <ctype.h>   // isprint
// #include <term.h>    // variable columns
#include <ncurses.h>
// #include <ncursesw/ncurses.h>

#include "str.h"
#include "process.h"
#include "conection.h"
// #include "rate.h"
#include "terminal.h"  // clear_cmd
#include "translate.h"
#include "show.h"
#include "sort.h"

// defined in main.c
extern bool view_conections;
extern process_t *processes;
extern uint32_t tot_process_act;

// defined un terminal.c
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

static int y, x;  // line, col getyx()
static int scroll_x = 0;
static int scroll_y = 1;
static int selected = 1;  // posição de linha do item selecionado
static int tot_rows;      // total linhas exibidas
static int sort_by = RATE_RX; // ordenação padrão

static int len_base_name, len_name;

// armazina a linha selecionada com seus atributos antes de estar "selecionada"
static chtype line_original[COLS_PAD];
// static chtype line_color[COLS_PAD];  // versão otimizada

static void
show_conections ( const process_t *process );

static void
show_header ();

void
show_process ( const process_t *processes, const size_t tot_process )
{
  // limpa a tela e o scrollback
  wclear ( pad );

  show_header ();

  tot_rows = 0;

  sort ( ( process_t * ) processes, tot_process, sort_by );

  wattron ( pad, A_BOLD );
  for ( size_t i = 0; i < tot_process; i++ )
    {
      // só exibe o processo se tiver fluxo de rede
      if ( processes[i].net_stat.tot_Bps_rx ||
           processes[i].net_stat.tot_Bps_tx )
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
                    "%*s %*s %*s %*s ",
                    RATE,
                    // "1023.69 kib/s",
                    processes[i].net_stat.tx_rate,
                    RATE,
                    // "1023.69 kib/s");
                    processes[i].net_stat.rx_rate,
                    RATE,
                    // "1023.69 kib/s",
                    processes[i].net_stat.tx_tot,
                    RATE,
                    // "1023.69 kib/s");
                    processes[i].net_stat.rx_tot );

          wprintw ( pad, "%s\n", processes[i].name );

          // /usr/bin/programa-nome
          len_base_name = strlen_space ( processes[i].name );

          // programa-nome
          len_name =
                  find_last_char ( processes[i].name, len_base_name, '/' ) + 1;

          getyx ( pad, y, x );
          // pinta linha inteira do nome do programa
          mvwchgat ( pad, y - 1, PROGRAM, -1, A_NORMAL, 1, NULL );
          // pinta somente o nome do programa
          mvwchgat ( pad,
                     y - 1,
                     PROGRAM + len_name,
                     len_base_name - len_name,
                     A_BOLD,
                     1,
                     NULL );
          wmove ( pad, y, x );

          if ( view_conections & ( processes[i].net_stat.avg_Bps_rx ||
                                   processes[i].net_stat.avg_Bps_tx ) )
            show_conections ( &processes[i] );
        }
    }
  wattroff ( pad, A_BOLD );

  // paint item selected
  if ( tot_rows )
    {
      if ( selected > tot_rows )
        selected = tot_rows;

      // salva conteudo da linha antes de pintar
      mvwinchnstr ( pad, selected, 0, line_original, COLS_PAD );

      // (re)pinta item selecionado
      // wmove(pad, selected, 0);
      int i = 0;
      while ( line_original[i] )
        {
          // line_color[i] = line_original[i];
          // line_color[i] &= A_CHARTEXT | A_ALTCHARSET;
          // line_color[i] |= COLOR_PAIR(4);
          // wpadch(pad, line_color[i++]);
          // versão otimizada
          waddch ( pad,
                   ( line_original[i++] & ( A_CHARTEXT | A_ALTCHARSET ) ) |
                           COLOR_PAIR ( 4 ) );
        }
    }

  // prefresh ( pad, 0, scroll_x, 0, 0, 0, COLS - 1 );  // atualiza cabeçalho
  pnoutrefresh ( pad, 0, scroll_x, 0, 0, 0, COLS - 1 );  // atualiza cabeçalho

  // prefresh ( pad, scroll_y, scroll_x, 1, 0, LINES - 1, COLS - 1 );
  pnoutrefresh ( pad, scroll_y, scroll_x, 1, 0, LINES - 1, COLS - 1 );

  // depois de todas as janelas verificadas, atualiza todas uma unica vez
  doupdate ();
}

static void
show_conections ( const process_t *process )
{
  // tuple ip:port <-> ip:port
  char *tuple;
  bool last_con = false;
  size_t i;

  // size_t index_act[process->total_conections];

  // pega o total de conexões no processo que estão ativas, e seus respectivos
  // indices
  // tot_con_act = get_con_active_in_process (
  //         index_act, process->conection, process->total_conections );

  wattron ( pad, A_DIM );
  for ( i = 0; i < process->total_conections; i++ )
    {
      tot_rows++;

      // se a proxima conexão estiver com estatisticas zeradas, essa é a ultima
      // conexão, as conexões são ordenadas de forma decrescente previamente
      if ( ( i < process->total_conections - 1 &&
             process->conection[i + 1].net_stat.avg_Bps_rx == 0 &&
             process->conection[i + 1].net_stat.avg_Bps_tx == 0 &&
             process->conection[i + 1].net_stat.tot_Bps_rx == 0 &&
             process->conection[i + 1].net_stat.tot_Bps_rx == 0 ) ||
           i == process->total_conections - 1 )
        last_con = true;

      // faz a tradução de ip:porta para nome-reverso:serviço
      tuple = translate ( &process->conection[i] );

      // waddch(pad, 'C' | A_INVIS);
      wprintw ( pad,
                " %*s %*ld %*ld %*s %*s ",
                PID,
                "",
                PPS,
                process->conection[i].net_stat.avg_pps_tx,
                PPS,
                process->conection[i].net_stat.avg_pps_rx,
                RATE,
                process->conection[i].net_stat.tx_rate,
                RATE,
                process->conection[i].net_stat.rx_rate );
      wprintw ( pad, "%*s", 29, "" );

      wattron ( pad, COLOR_PAIR ( 1 ) );
      if ( !last_con )
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

      if ( last_con )
        break;
    }
  // se teve conexões exibidas
  if ( i )
    {
      waddch ( pad, '\n' );
      tot_rows++;
    }

  wattroff ( pad, A_DIM );
}

static void
show_header ( void )
{
  // wprintw ( pad,
  //           " %*s %*s %*s     %s     %s       %s      %s     %s\n",
  //           PID,
  //           "PID",
  //           PPS,
  //           "PPS TX",
  //           PPS,
  //           "PPS RX",
  //           "RATE TX",
  //           "RATE RX",
  //           "TOTAL TX",
  //           "TOTAL RX",
  //           "PROGRAM" );
  wprintw ( pad,
            " %*s %*s %*s     %s     %s       %s      %s     %s\n",
            PID,
            "PID",
            PPS,
            "PPS TX",
            PPS,
            "PPS RX",
            "RATE TX",
            "RATE RX",
            "TOTAL TX",
            "TOTAL RX",
            "PROGRAM" );
  // paint line header
  getyx ( pad, y, x );
  mvwchgat ( pad, 0, 0, -1, 0, 2, NULL );
  wmove ( pad, y, x );
}

void
ui_tick ()
{
  int ch;
  int i = 0;

  while ( ( ch = wgetch ( pad ) ) != ERR )
    {
      switch ( ch )
        {
          // scroll horizontal
          case KEY_RIGHT:
            if ( scroll_x < COLS_PAD )
              {
                scroll_x += 5;
                prefresh ( pad, 0, scroll_x, 0, 0, LINES - 1, COLS - 1 );
              }
            else
              beep ();

            break;
          case KEY_LEFT:
            if ( scroll_x > 0 )
              {
                scroll_x -= 5;
                prefresh ( pad, 0, scroll_x, 0, 0, LINES - 1, COLS - 1 );
              }
            else
              beep ();

            break;
          case KEY_DOWN:
            if ( selected + 1 <= tot_rows )
              {
                selected++;
                if ( selected >= LINES )
                  scroll_y++;

                // restaura linha atual
                mvwaddchnstr ( pad, selected - 1, 0, line_original, COLS_PAD );

                // salva linha que sera marcada/selecionada (antes de estar
                // pintada)
                mvwinchnstr ( pad, selected, 0, line_original, COLS_PAD );

                // pinta a linha selecionada
                i = 0;
                while ( line_original[i] )
                  {
                    // line_color[i] = line_original[i];
                    //  // retira atributos
                    // line_color[i] &= A_CHARTEXT | A_ALTCHARSET;
                    // line_color[i] |= COLOR_PAIR(4);  // adiciona atributos
                    // novo
                    waddch ( pad,
                             ( line_original[i++] &
                               ( A_CHARTEXT | A_ALTCHARSET ) ) |
                                     COLOR_PAIR ( 4 ) );
                  }

                // atualiza tela
                pnoutrefresh (
                        pad, scroll_y, scroll_x, 1, 0, LINES - 1, COLS - 1 );
                doupdate ();
              }
            else
              beep ();

            break;
          case KEY_UP:
            if ( selected - 1 >= 1 )
              {
                selected--;
                if ( scroll_y > 1 && selected <= LINES )
                  scroll_y--;

                // restaura linha atual
                mvwaddchstr ( pad, selected + 1, 0, line_original );

                // salva linha que sera marcada/selecionada (antes de estar
                // pintada)
                mvwinchstr ( pad, selected, 0, line_original );

                i = 0;
                while ( line_original[i] )
                  {
                    // line_color[i] = line_original[i];
                    // line_color[i] &= A_CHARTEXT | A_ALTCHARSET;
                    // line_color[i] |= COLOR_PAIR(4);
                    // waddch(pad, line_color[i++] );
                    waddch ( pad,
                             ( line_original[i++] &
                               ( A_CHARTEXT | A_ALTCHARSET ) ) |
                                     COLOR_PAIR ( 4 ) );
                  }

                // atualiza tela
                pnoutrefresh (
                        pad, scroll_y, scroll_x, 1, 0, LINES - 1, COLS - 1 );
                doupdate ();
              }
            else
              beep ();

            break;
          case 's':
            sort_by = (sort_by + 1 <= COLS_TO_SORT) ? sort_by + 1 : 1;
            break;
          case 'q':
            exit(EXIT_SUCCESS);
        }
    }
}
