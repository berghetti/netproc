
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
#include <string.h>  // strlen
#include <net/if.h>  // if_indextoname, IF_NAMESIZE
#include <ncurses.h>

#include "str.h"
#include "timer.h"
#include "processes.h"
#include "connection.h"
#include "color.h"
#include "m_error.h"
#include "translate.h"
#include "tui.h"
#include "usage.h"
#include "sort.h"
#include "rate.h"  // type nstats_t
#include "human_readable.h"
#include "pid.h"
#include "macro_util.h"

#define PORTLEN 5  // strlen("65535")

// space between columns
#define PPS 6
#define J_RATE 13

// espaçamento da estatistica até a tupla
#define TUPLE 16 - IF_NAMESIZE

// linha que começa a ser exibido os programas ativos
#define LINE_START 4

// tamanho fixo de caracteres até a coluna program
// menos a caluna PID que é variavel
#define PROGRAM 71

// FIXME: check this values
#define START_NAME_PROGRAM 64
#define MIN_LINES_PAD 64

#define MIN_COLS_PAD PROGRAM + START_NAME_PROGRAM

static WINDOW *pad = NULL;
static int *color_scheme;

// armazina a linha selecionada com seus atributos antes de estar "selecionada"
static chtype *line_original = NULL;  // size is equal cur_cols

static int sort_by = RATE_RX;  // ordenação padrão
static int scroll_x = 0;
static int scroll_y = LINE_START + 1;
static int selected = LINE_START + 1;  // posição de linha do item selecionado

static int tot_rows;  // total linhas exibidas

static int tot_proc_act = 0;  // total de processos com conexão ativa

// statistics total in current time
static nstats_t cur_rate_tx, cur_rate_rx;
static nstats_t cur_pps_tx, cur_pps_rx;

// total lines and cols current on pad
static int cur_cols;
static int cur_lines;

static int max_digits_pid;

static void
paint_selected ( void )
{
  // save current line selected without colors to line_original
  // after this is restored
  mvwinchnstr ( pad, selected, 0, line_original, cur_cols - 1 );
  line_original[cur_cols - 1] = ' ';  // remove '\n' end of line

  // print current line selected with colors
  for ( int i = 0; i < cur_cols; i++ )
    waddch ( pad,
             ( line_original[i] & ( A_CHARTEXT | A_ALTCHARSET ) ) |
                     color_scheme[SELECTED_L] );
}

static void
show_resume ( const struct config_op *co )
{
  char rate_tx[LEN_STR_RATE], rate_rx[LEN_STR_RATE];

  human_readable ( rate_tx, LEN_STR_RATE, cur_rate_tx, RATE );
  human_readable ( rate_rx, LEN_STR_RATE, cur_rate_rx, RATE );

  wattrset ( pad, color_scheme[RESUME] );
  mvwprintw ( pad, 0, 1, PROG_NAME " - " PROG_VERSION "\n" );

  wmove ( pad, 2, 1 );
  wclrtoeol ( pad );  // erase the current line
  wprintw ( pad, "Running: " );
  wattrset ( pad, color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%s", msec2clock ( co->running ) );

  wattrset ( pad, color_scheme[RESUME] );
  mvwprintw ( pad, 2, 25, "pps tx: " );
  wattrset ( pad, color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%ld", cur_pps_tx );
  wattrset ( pad, color_scheme[RESUME] );
  mvwprintw ( pad, 2, 40, "rate tx: " );
  wattrset ( pad, color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%s", rate_tx );

  wattrset ( pad, color_scheme[RESUME] );
  wmove ( pad, 3, 1 );
  wprintw ( pad, "Processes: " );
  wclrtoeol ( pad );  // erase the current line
  wattrset ( pad, color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%d", tot_proc_act );

  wattrset ( pad, color_scheme[RESUME] );
  mvwprintw ( pad, 3, 25, "pps rx: " );
  wattrset ( pad, color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%ld", cur_pps_rx );
  wattrset ( pad, color_scheme[RESUME] );
  mvwprintw ( pad, 3, 40, "rate rx: " );
  wattrset ( pad, color_scheme[RESUME_VALUE] );
  wprintw ( pad, "%s", rate_rx );

  wattrset ( pad, color_scheme[RESET] );

  // update all resume
  pnoutrefresh ( pad, 0, 0, 0, 0, LINE_START - 1, COLS - 1 );
}

static void
show_header ( const struct config_op *co )
{
  show_resume ( co );

  wmove ( pad, LINE_START, 0 );  // move first line

  wattrset ( pad,
             ( sort_by == S_PID ) ? color_scheme[SELECTED_H]
                                  : color_scheme[HEADER] );
  wprintw ( pad, "%*s ", max_digits_pid, "PID" );

  wattrset ( pad,
             ( sort_by == PPS_TX ) ? color_scheme[SELECTED_H]
                                   : color_scheme[HEADER] );
  wprintw ( pad, "%*s ", PPS, "PPS TX" );

  wattrset ( pad,
             ( sort_by == PPS_RX ) ? color_scheme[SELECTED_H]
                                   : color_scheme[HEADER] );
  wprintw ( pad, "%*s", PPS, "PPS RX" );

  wattrset ( pad,
             ( sort_by == RATE_TX ) ? color_scheme[SELECTED_H]
                                    : color_scheme[HEADER] );
  wprintw ( pad, "    %s   ", "RATE TX" );

  wattrset ( pad,
             ( sort_by == RATE_RX ) ? color_scheme[SELECTED_H]
                                    : color_scheme[HEADER] );
  wprintw ( pad, "    %s   ", "RATE RX" );

  wattrset ( pad,
             ( sort_by == TOT_TX ) ? color_scheme[SELECTED_H]
                                   : color_scheme[HEADER] );
  wprintw ( pad, "    %s    ", "TOTAL TX" );

  wattrset ( pad,
             ( sort_by == TOT_RX ) ? color_scheme[SELECTED_H]
                                   : color_scheme[HEADER] );
  wprintw ( pad, "  %s   ", "TOTAL RX" );

  // paint to the end of line
  wattrset ( pad, color_scheme[HEADER] );
  wprintw ( pad,
            "%*s",
            -( cur_cols - ( PROGRAM + max_digits_pid ) ),
            "PROGRAM" );

  wattrset ( pad, color_scheme[RESET] );

  // update only line header
  pnoutrefresh ( pad,
                 LINE_START,
                 scroll_x,
                 LINE_START,
                 scroll_x,
                 LINE_START,
                 COLS - 1 );
}

static WINDOW *
create_pad ( const int l, const int c )
{
  WINDOW *p = newpad ( l, c );

  if ( p )
    {
      nodelay ( p, TRUE );  // no gelay getch()
      keypad ( p, TRUE );   // get arrow key
      curs_set ( 0 );       // cursor invisible
    }

  return p;
}

static void
resize_pad ( const int l, const int c )
{
  if ( l < cur_lines && c < cur_cols )
    return;

  cur_lines = MAX ( l, cur_lines );
  cur_cols = MAX ( c, cur_cols );

  wresize ( pad, cur_lines, cur_cols );

  size_t new_size = cur_cols * sizeof ( *line_original ) + 1;  // +1 null byte
  void *tmp = realloc ( line_original, new_size );
  if ( !tmp )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return;
    }

  line_original = tmp;
}

static void
show_connections ( const process_t *process, const struct config_op *co )
{
  resize_pad ( process->total_conections + tot_rows, 0 );

  for ( size_t i = 0; i < process->total_conections; i++ )
    {
      bool last_con = false;
      tot_rows++;

      // se a proxima conexão estiver com estatisticas zeradas, essa é a ultima
      // conexão, as conexões são ordenadas de forma decrescente previamente
      if ( ( i < process->total_conections - 1 &&
             process->conections[i + 1]->net_stat.avg_Bps_rx == 0 &&
             process->conections[i + 1]->net_stat.avg_Bps_tx == 0 ) ||
           i == process->total_conections - 1 )
        last_con = true;

      char *tuple = translate ( process->conections[i], co );

      char tx_rate[LEN_STR_RATE], rx_rate[LEN_STR_RATE];

      human_readable ( tx_rate,
                       sizeof tx_rate,
                       process->conections[i]->net_stat.avg_Bps_tx,
                       RATE );

      human_readable ( rx_rate,
                       sizeof rx_rate,
                       process->conections[i]->net_stat.avg_Bps_rx,
                       RATE );

      wattrset ( pad, color_scheme[CONECTIONS] );
      wprintw ( pad,
                "%*s %*ld %*ld %*s %*s ",
                max_digits_pid,
                "",
                PPS,
                process->conections[i]->net_stat.avg_pps_tx,
                PPS,
                process->conections[i]->net_stat.avg_pps_rx,
                J_RATE,
                tx_rate,
                J_RATE,
                rx_rate );

      char iface_buff[IF_NAMESIZE];
      char *iface;

      if ( if_indextoname ( process->conections[i]->if_index, iface_buff ) )
        iface = iface_buff;
      else
        iface = "";

      wprintw ( pad,
                "%*s %*s",
                IF_NAMESIZE,
                iface,
                -11,
                ( process->conections[i]->tuple.l4.protocol == IPPROTO_TCP )
                        ? "(tcp)"
                        : "(udp)" );

      // space tuple
      wprintw ( pad, "%*s", TUPLE, "" );

      wattrset ( pad, color_scheme[TREE] );
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

      wattrset ( pad, color_scheme[CONECTIONS] );
      wprintw ( pad, " %s\n", tuple );

      if ( last_con )
        {
          tot_rows++;
          waddch ( pad, '\n' );
          break;
        }
    }

  wattrset ( pad, color_scheme[RESET] );
}

static void
set_lines_cols ( void )
{
  cur_cols = MAX ( COLS, MIN_COLS_PAD );
  cur_lines = MAX ( LINES, MIN_LINES_PAD );
}

int
tui_init ( const struct config_op *co )
{
  initscr ();
  cbreak ();  // disable buffering to get keypad
  noecho ();

  set_lines_cols ();
  size_t size = cur_cols * sizeof ( *line_original ) + 1;  // + 1 null byte
  line_original = malloc ( size );
  if ( !line_original )
    return 0;

  pad = create_pad ( cur_lines, cur_cols );
  if ( !pad )
    return 0;

  color_scheme = get_color_scheme ( co->color_scheme );
  max_digits_pid = get_max_digits_pid ();

  show_header ( co );
  doupdate ();

  return 1;
}

void
tui_show ( const struct processes *processes, const struct config_op *co )
{
  tot_rows = LINE_START;
  tot_proc_act = 0;
  cur_rate_tx = cur_rate_rx = cur_pps_tx = cur_pps_rx = 0;

  static int tot_cols = 0;

  sort ( processes->proc, processes->total, sort_by, co );

  wmove ( pad, LINE_START + 1, 0 );  // move second line after header

  for ( size_t i = 0; i < processes->total; i++ )
    {
      // process_t *process = *proc;
      process_t *process = processes->proc[i];
      //
      if ( !( process->net_stat.tot_Bps_rx || process->net_stat.tot_Bps_tx ) &&
           !co->verbose )
        continue;

      tot_rows++;
      tot_proc_act++;

      // update total show in resume
      cur_rate_tx += process->net_stat.avg_Bps_tx;
      cur_rate_rx += process->net_stat.avg_Bps_rx;

      cur_pps_tx += process->net_stat.avg_pps_tx;
      cur_pps_rx += process->net_stat.avg_pps_rx;

      // "/usr/bin/program-name --any_parameters"
      size_t len_full_name = strlen ( process->name );

      // +1 because'\n'
      tot_cols = MAX ( ( size_t ) tot_cols,
                       len_full_name + PROGRAM + max_digits_pid + 1 );

      resize_pad ( 0, tot_cols );

      char tx_rate[LEN_STR_RATE], rx_rate[LEN_STR_RATE];
      char tx_tot[LEN_STR_TOTAL], rx_tot[LEN_STR_TOTAL];
      human_readable ( tx_rate,
                       sizeof tx_rate,
                       process->net_stat.avg_Bps_tx,
                       RATE );

      human_readable ( rx_rate,
                       sizeof rx_rate,
                       process->net_stat.avg_Bps_rx,
                       RATE );

      human_readable ( tx_tot,
                       sizeof tx_tot,
                       process->net_stat.tot_Bps_tx,
                       TOTAL );

      human_readable ( rx_tot,
                       sizeof rx_tot,
                       process->net_stat.tot_Bps_rx,
                       TOTAL );

      wprintw ( pad,
                "%*d %*ld %*ld %*s %*s %*s %*s ",
                max_digits_pid,
                process->pid,
                PPS,
                process->net_stat.avg_pps_tx,
                PPS,
                process->net_stat.avg_pps_rx,
                J_RATE,
                tx_rate,
                J_RATE,
                rx_rate,
                J_RATE,
                tx_tot,
                J_RATE,
                rx_tot );

      // "/usr/bin/program-name"
      size_t len_path_name = strlen_space ( process->name );

      // simulate end of string to index_last_char
      char tmp = process->name[len_path_name];
      process->name[len_path_name] = '\0';

      // "program-name"
      ssize_t start_name = index_last_char ( process->name, '/' );

      process->name[len_path_name] = tmp;

      // if start_name == -1, name program starting in 0 position, else skip '/'
      start_name++;

      for ( size_t j = 0; j < len_full_name; j++ )
        {
          chtype ch = process->name[j];

          if ( j < ( size_t ) start_name )
            ch |= color_scheme[PATH_PROG];
          else if ( j < len_path_name )
            ch |= color_scheme[NAME_PROG];
          else
            ch |= color_scheme[PARAM_PROG];

          waddch ( pad, ch );
        }

      waddch ( pad, '\n' );

      // option -c and process with traffic at the moment
      if ( co->view_conections &&
           ( process->net_stat.avg_Bps_rx || process->net_stat.avg_Bps_tx ) )
        show_connections ( process, co );
    }

  // clear lines begin cursor end screen, "replace" wclear()
  wclrtobot ( pad );

  // paint item selected
  if ( tot_rows > LINE_START + 1 )
    {
      if ( selected > tot_rows )
        selected = tot_rows;

      paint_selected ();
    }

  show_header ( co );

  pnoutrefresh ( pad,
                 scroll_y,
                 scroll_x,
                 LINE_START + 1,
                 0,
                 LINES - 1,
                 COLS - 1 );

  // full refresh
  doupdate ();
}

static void
restore_line ( int line )
{
  mvwaddchnstr ( pad, line, 0, line_original, cur_cols );
}

// handle input of user while program is running
int
tui_handle_input ( const struct config_op *co )
{
  int ch;

  if ( !pad )
    return P_CONTINE;

  while ( ( ch = wgetch ( pad ) ) != ERR )
    {
      switch ( ch )
        {
          // scroll horizontal
          case KEY_RIGHT:
            for ( int i = 5; i >= 0; i-- )
              {
                if ( i == 0 )
                  {
                    beep ();
                    break;
                  }
                else if ( scroll_x + i <= cur_cols - COLS )
                  {
                    scroll_x += i;

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
                    break;
                  }
              }
            break;
          case KEY_LEFT:
            for ( int i = 5; i >= 0; i-- )
              {
                if ( i == 0 )
                  {
                    beep ();
                    break;
                  }
                else if ( scroll_x - i >= 0 )
                  {
                    scroll_x -= i;

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
                    break;
                  }
              }
            break;
          case KEY_DOWN:
            if ( ++selected <= tot_rows )
              {
                if ( selected >= LINES - 1 )
                  scroll_y++;

                // restore current line
                restore_line ( selected - 1 );

                paint_selected ();

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

                // restore current line
                restore_line ( selected + 1 );

                paint_selected ();

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
            return P_EXIT;
        }
    }

  return P_CONTINE;
}

void
tui_free ( void )
{
  if ( pad )
    delwin ( pad );

  curs_set ( 1 );  // restore cursor
  endwin ();
  free ( line_original );
}
