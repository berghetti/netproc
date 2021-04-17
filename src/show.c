
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

#include <stdbool.h>
#include <string.h>    // strlen
#include <net/if.h>    // if_indextoname, IF_NAMESIZE
#include <linux/in.h>  // IPPROTO_TCP
#include <ncurses.h>

#include "str.h"
#include "timer.h"
#include "process.h"
#include "conection.h"
#include "color.h"
#include "m_error.h"
#include "translate.h"
#include "terminal.h"
#include "show.h"
#include "usage.h"
#include "sort.h"
#include "rate.h"  // type nstats_t
#include "human_readable.h"

// tamanho representação textual porta da camada de transporte
#define PORTLEN 5  // 65535

// space between columns
// #define PID -5  // negative == justify left
#define PPS 6
#define J_RATE 13

// espaçamento da estatistica até a tupla
#define TUPLE 16 - IF_NAMESIZE

// linha que começa a ser exibido os programas ativos
#define LINE_START 4

// FIXME: const value no ok
// armazina a linha selecionada com seus atributos antes de estar "selecionada"
static chtype line_original[1000] = { 0 };

static int sort_by = RATE_RX;  // ordenação padrão
static int scroll_x = 0;
static int scroll_y = LINE_START + 1;
static int selected = LINE_START + 1;  // posição de linha do item selecionado
static int tot_rows;                   // total linhas exibidas

static int tot_proc_act = 0;  // total de processos com conexão ativa

// statistics total in current time
static nstats_t cur_rate_tx, cur_rate_rx;
static nstats_t cur_pps_tx, cur_pps_rx;

// total lines and cols current on pad
static int cur_cols;
static int cur_lines;

#define MAX( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )

// it is not possible to initialize the variable at compile time
static void
set_lines_cols ( void )
{
  cur_cols = MAX ( COLS, MIN_COLS_PAD );
  cur_lines = MAX ( LINES, MIN_LINES_PAD );
}

static void
paint_selected ( const struct config_op *co )
{
  for ( int i = 0; i < cur_cols; i++ )
    waddch ( pad,
             ( line_original[i] & ( A_CHARTEXT | A_ALTCHARSET ) ) |
                     co->color_scheme[SELECTED_L] );
}

static void
show_resume ( const struct config_op *co )
{
  char rate_tx[LEN_STR_RATE], rate_rx[LEN_STR_RATE];

  human_readable ( rate_tx, LEN_STR_RATE, cur_rate_tx, RATE );
  human_readable ( rate_rx, LEN_STR_RATE, cur_rate_rx, RATE );

  wattrset ( pad, co->color_scheme[RESUME] );
  mvwprintw ( pad, 0, 1, PROG_NAME " - " PROG_VERSION "\n" );

  wmove ( pad, 2, 1 );
  wclrtoeol ( pad );  // erase the current line
  wprintw ( pad, "Running: " );
  wattrset ( pad, co->color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%s", sec2clock ( ( uint64_t ) co->running ) );

  wattrset ( pad, co->color_scheme[RESUME] );
  mvwprintw ( pad, 2, 25, "pps tx: " );
  wattrset ( pad, co->color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%ld", cur_pps_tx );
  wattrset ( pad, co->color_scheme[RESUME] );
  mvwprintw ( pad, 2, 40, "rate tx: " );
  wattrset ( pad, co->color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%s", rate_tx );

  wattrset ( pad, co->color_scheme[RESUME] );
  wmove ( pad, 3, 1 );
  wprintw ( pad, "Processes: " );
  wclrtoeol ( pad );  // erase the current line
  wattrset ( pad, co->color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%d", tot_proc_act );

  wattrset ( pad, co->color_scheme[RESUME] );
  mvwprintw ( pad, 3, 25, "pps rx: " );
  wattrset ( pad, co->color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%d", cur_pps_rx );
  wattrset ( pad, co->color_scheme[RESUME] );
  mvwprintw ( pad, 3, 40, "rate rx: " );
  wattrset ( pad, co->color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%s", rate_rx );

  wattrset ( pad, co->color_scheme[RESET] );

  pnoutrefresh ( pad, 0, 0, 0, 0, LINE_START - 1, COLS - 1 );
}

static void
show_header ( const struct config_op *co )
{
  show_resume ( co );

  wmove ( pad, LINE_START, 0 );  // move first line

  wattrset ( pad,
             ( sort_by == S_PID ) ? co->color_scheme[SELECTED_H]
                                  : co->color_scheme[HEADER] );
  wprintw ( pad, "%*s ", co->max_digits_pid, "PID" );

  wattrset ( pad,
             ( sort_by == PPS_TX ) ? co->color_scheme[SELECTED_H]
                                   : co->color_scheme[HEADER] );
  wprintw ( pad, "%*s ", PPS, "PPS TX" );

  wattrset ( pad,
             ( sort_by == PPS_RX ) ? co->color_scheme[SELECTED_H]
                                   : co->color_scheme[HEADER] );
  wprintw ( pad, "%*s", PPS, "PPS RX" );

  wattrset ( pad,
             ( sort_by == RATE_TX ) ? co->color_scheme[SELECTED_H]
                                    : co->color_scheme[HEADER] );
  wprintw ( pad, "    %s   ", "RATE TX" );

  wattrset ( pad,
             ( sort_by == RATE_RX ) ? co->color_scheme[SELECTED_H]
                                    : co->color_scheme[HEADER] );
  wprintw ( pad, "    %s   ", "RATE RX" );

  wattrset ( pad,
             ( sort_by == TOT_TX ) ? co->color_scheme[SELECTED_H]
                                   : co->color_scheme[HEADER] );
  wprintw ( pad, "    %s    ", "TOTAL TX" );

  wattrset ( pad,
             ( sort_by == TOT_RX ) ? co->color_scheme[SELECTED_H]
                                   : co->color_scheme[HEADER] );
  wprintw ( pad, "  %s   ", "TOTAL RX" );

  wattrset ( pad, co->color_scheme[HEADER] );
  // paint to the end of line
  wprintw ( pad, "%*s", -( cur_cols ), "PROGRAM" );

  wattrset ( pad, co->color_scheme[RESET] );

  // update header
  pnoutrefresh ( pad,
                 LINE_START,
                 scroll_x,
                 LINE_START,
                 scroll_x,
                 LINE_START,
                 COLS - 1 );
}

static void
show_conections ( const process_t *restrict process,
                  const struct config_op *restrict co )
{
  // tuple ip:port <-> ip:port
  char *tuple;
  bool last_con = false;
  size_t i;
  char iface_buff[IF_NAMESIZE];
  char *iface;

  char tx_rate[LEN_STR_RATE], rx_rate[LEN_STR_RATE];

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
      tuple = translate ( &process->conection[i], co );

      human_readable ( tx_rate,
                       sizeof tx_rate,
                       process->conection[i].net_stat.avg_Bps_tx,
                       RATE );

      human_readable ( rx_rate,
                       sizeof rx_rate,
                       process->conection[i].net_stat.avg_Bps_rx,
                       RATE );

      wattrset ( pad, co->color_scheme[CONECTIONS] );
      wprintw ( pad,
                "%*s %*ld %*ld %*s %*s ",
                co->max_digits_pid,
                "",
                PPS,
                process->conection[i].net_stat.avg_pps_tx,
                PPS,
                process->conection[i].net_stat.avg_pps_rx,
                J_RATE,
                tx_rate,
                J_RATE,
                rx_rate );

      if ( if_indextoname ( process->conection[i].if_index, iface_buff ) )
        iface = iface_buff;
      else
        iface = "";

      // wprintw ( pad, "             %*s", -( IF_NAMESIZE ), iface );
      wprintw ( pad,
                "%*s %*s",
                IF_NAMESIZE,
                iface,
                -11,
                ( process->conection[i].protocol == IPPROTO_TCP ) ? "(tcp)"
                                                                  : "(udp)" );

      // space tuple
      wprintw ( pad, "%*s", TUPLE, "" );

      wattrset ( pad, co->color_scheme[TREE] );
      if ( !last_con )
        {
          waddch ( pad, ACS_LTEE );   // ├
          waddch ( pad, ACS_HLINE );  // ─
        }
      else
        {                                // last conection
          waddch ( pad, ACS_LLCORNER );  // └
          waddch ( pad, ACS_HLINE );     // ─
        }

      wattrset ( pad, co->color_scheme[CONECTIONS] );
      wprintw ( pad, " %s\n", tuple );

      if ( last_con )
        break;
    }
  // se teve conexões exibidas, pula uma linha
  if ( last_con )
    {
      waddch ( pad, '\n' );
      tot_rows++;
    }

  wattrset ( pad, co->color_scheme[RESET] );
}

void
start_ui ( const struct config_op *co )
{
  set_lines_cols ();
  // line_original = malloc(cur_cols * sizeof(chtype));
  // if (!line_original)
  ERROR_DEBUG ( "%s", strerror ( errno ) );
  show_header ( co );
  doupdate ();
}

void
show_process ( const process_t *restrict processes,
               const size_t tot_process,
               const struct config_op *restrict co )
{
  {
    int temp_cols = co->max_name_process + PROGRAM;
    bool need_resize_pad = false;

    if ( temp_cols > cur_cols )
      {
        cur_cols = temp_cols;
        need_resize_pad = true;
      }
    if ( co->tot_rows > ( uint32_t ) cur_lines )
      {
        cur_lines = co->tot_rows;
        need_resize_pad = true;
      }
    if ( need_resize_pad )
      {
        resize_pad ( cur_lines, cur_cols );
        show_header ( co );
      }
  }

  tot_rows = LINE_START;

  tot_proc_act = 0;

  cur_rate_tx = cur_rate_rx = cur_pps_tx = cur_pps_rx = 0;

  char tx_rate[LEN_STR_RATE], rx_rate[LEN_STR_RATE];
  char tx_tot[LEN_STR_TOTAL], rx_tot[LEN_STR_TOTAL];

  sort ( ( process_t * ) processes, tot_process, sort_by, co );

  wmove ( pad, LINE_START + 1, 0 );  // move second line after header
  for ( size_t i = 0; i < tot_process; i++ )
    {
      // só exibe o processo se tiver fluxo de rede
      // ou se modo verboso ligado
      if ( !co->verbose && !( processes[i].net_stat.tot_Bps_rx ||
                              processes[i].net_stat.tot_Bps_tx ) )
        continue;

      tot_rows++;
      tot_proc_act++;

      // update total show in resume
      cur_rate_tx += processes[i].net_stat.avg_Bps_tx;
      cur_rate_rx += processes[i].net_stat.avg_Bps_rx;

      cur_pps_tx += processes[i].net_stat.avg_pps_tx;
      cur_pps_rx += processes[i].net_stat.avg_pps_rx;

      human_readable (
              tx_rate, sizeof tx_rate, processes[i].net_stat.avg_Bps_tx, RATE );

      human_readable (
              rx_rate, sizeof rx_rate, processes[i].net_stat.avg_Bps_rx, RATE );

      human_readable (
              tx_tot, sizeof tx_tot, processes[i].net_stat.tot_Bps_tx, TOTAL );

      human_readable (
              rx_tot, sizeof rx_tot, processes[i].net_stat.tot_Bps_rx, TOTAL );

      wprintw ( pad,
                "%*d %*ld %*ld %*s %*s %*s %*s ",
                co->max_digits_pid,
                processes[i].pid,
                PPS,
                processes[i].net_stat.avg_pps_tx,
                PPS,
                processes[i].net_stat.avg_pps_rx,
                J_RATE,
                tx_rate,
                J_RATE,
                rx_rate,
                J_RATE,
                tx_tot,
                J_RATE,
                rx_tot );

      // wprintw ( pad, "%s", processes[i].name );

      // "/usr/bin/programa-nome" --any_parameters
      size_t len_base_name = strlen_space ( processes[i].name );

      // programa-nome
      size_t len_name =
              find_last_char ( processes[i].name, len_base_name, '/' ) + 1;

      for ( size_t j = 0; j < processes[i].len_name; j++ )
        {
          if ( j >= len_name && j < len_base_name )
            // destaca somente o nome do programa
            waddch ( pad,
                     processes[i].name[j] | co->color_scheme[NAME_PROG_BOLD] );
          else
            // pinta todo o caminho do programa e parametros
            waddch ( pad, processes[i].name[j] | co->color_scheme[NAME_PROG] );
        }

      waddch ( pad, '\n' );

      // option -c and process with traffic at the moment
      if ( co->view_conections & ( processes[i].net_stat.avg_Bps_rx ||
                                   processes[i].net_stat.avg_Bps_tx ) )

        show_conections ( &processes[i], co );
    }

  // clear lines begin cursor end screen, replace wclear()
  wclrtobot ( pad );

  // paint item selected
  if ( tot_rows > LINE_START )
    {
      if ( selected > tot_rows )
        selected = tot_rows;

      // salva conteudo da linha antes de pintar
      mvwinchnstr ( pad, selected, 0, line_original, cur_cols );

      // (re)pinta item selecionado
      paint_selected ( co );
    }

  // wresize(pad, LINES, COLS);
  // update line header
  pnoutrefresh (
          pad, LINE_START, scroll_x, LINE_START, 0, LINE_START, COLS - 1 );

  pnoutrefresh (
          pad, scroll_y, scroll_x, LINE_START + 1, 0, LINES - 1, COLS - 1 );

  show_resume ( co );

  // depois de todas as janelas atualizadas, imprime todas uma unica vez
  doupdate ();
}

// handle input of user while program is running
int
running_input ( const struct config_op *co )
{
  int ch;

  while ( ( ch = wgetch ( pad ) ) != ERR )
    {
      switch ( ch )
        {
          // scroll horizontal
          case KEY_RIGHT:
            if ( scroll_x + 5 < cur_cols - COLS )
              {
                scroll_x = ( scroll_x + 5 <= cur_cols - COLS )
                                   ? scroll_x + 5
                                   : cur_cols - COLS;

                // update header
                pnoutrefresh ( pad,
                               LINE_START,
                               scroll_x,
                               LINE_START,
                               0,
                               LINE_START,
                               COLS - 1 );

                pnoutrefresh ( pad,
                               scroll_y,
                               scroll_x,
                               LINE_START + 1,
                               0,
                               LINES - 1,
                               COLS - 1 );
                doupdate ();
              }
            else
              beep ();

            break;
          case KEY_LEFT:
            if ( scroll_x > 0 )
              {
                scroll_x = ( scroll_x - 5 >= 0 ) ? scroll_x - 5 : 0;

                // update header
                pnoutrefresh ( pad,
                               LINE_START,
                               scroll_x,
                               LINE_START,
                               0,
                               LINE_START,
                               COLS - 1 );

                pnoutrefresh ( pad,
                               scroll_y,
                               scroll_x,
                               LINE_START + 1,
                               0,
                               LINES - 1,
                               COLS - 1 );
                doupdate ();
              }
            else
              beep ();

            break;
          case KEY_DOWN:
            if ( ++selected <= tot_rows )
              {
                if ( selected >= LINES - 1 )
                  scroll_y++;

                // restaura linha atual
                mvwaddchstr ( pad, selected - 1, 0, line_original );

                // salva linha que sera marcada/selecionada (antes de estar
                // pintada)
                mvwinchnstr ( pad, selected, 0, line_original, cur_cols );

                // pinta a linha selecionada
                paint_selected ( co );

                // atualiza tela
                prefresh ( pad,
                           scroll_y,
                           scroll_x,
                           LINE_START + 1,
                           0,
                           LINES - 1,
                           COLS - 1 );
              }
            else
              {
                selected--;
                beep ();
              }

            break;
          case KEY_UP:
            if ( --selected > LINE_START )
              {
                if ( scroll_y > LINE_START + 1 )
                  scroll_y--;

                // restaura linha atual
                mvwaddchstr ( pad, selected + 1, 0, line_original );

                // salva linha que sera marcada/selecionada (antes de estar
                // pintada)
                mvwinchnstr ( pad, selected, 0, line_original, cur_cols );

                paint_selected ( co );

                // atualiza tela
                prefresh ( pad,
                           scroll_y,
                           scroll_x,
                           LINE_START + 1,
                           0,
                           LINES - 1,
                           COLS - 1 );
              }
            else
              {
                selected++;
                beep ();
              }

            break;
          case 's':
          case 'S':
            sort_by = ( sort_by + 1 ) % COLS_TO_SORT;
            show_header ( co );
            doupdate ();
            break;
          case 'q':
          case 'Q':
            // exit ( EXIT_SUCCESS );
            return P_EXIT;
        }
    }

  return P_CONTINE;
}
