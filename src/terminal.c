
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

#include <term.h>    // setupterm, tputs, tigetstr
#include <unistd.h>  // STDOUT_FILENO
#include <ncurses.h>
// #include <ncursesw/ncurses.h>

#include "color.h"
#include "show.h"
#include "m_error.h"

WINDOW *pad;

// static char *E3;

// carrega informações do terminal a associa a stdout
void
setup_terminal ( void )
{
  // ainda necessario para exibir correr em mensagens de erro
  int err;
  if ( setupterm ( NULL, STDOUT_FILENO, &err ) == -1 )
    {
      switch ( err )
        {
          case 1:
            fatal_error ( "terminal is hardcopy" );
            break;
          case -1:
            fatal_error ( "no terminfo database" );
            break;
          case 0:
            fatal_error ( "unknown terminal" );
        }
    }

  // maneira terminfo
  // codigo de escape para limpar scrollback
  // E3 = tigetstr ( "E3" );
  //
  // tputs ( cursor_invisible, 1, putchar );
}

void
init_ui ()
{
  initscr ();
  cbreak ();  // disable buffering to get keypad
  noecho ();

  pad = newpad ( LINES * 2, COLS_PAD );

  nodelay ( pad, TRUE );  // no gelay getch()
  keypad ( pad, TRUE );   // get arrow key

  idlok ( pad, TRUE );
  scrollok ( pad, TRUE );

  curs_set ( 0 );  // cursor invisible

  define_color_scheme ();
  // start_color ();
  // use_default_colors ();

  // color foreground, background
  // init_pair ( 1, COLOR_CYAN, -1 );            // color tree, name program
  // init_pair ( 2, COLOR_BLACK, COLOR_GREEN );  // color header
  // init_pair ( 3, -1, COLOR_BLACK );
  // init_pair ( 4, COLOR_BLACK, COLOR_CYAN );  // line selected, column sorted
}

void
restore_terminal ( void )
{
  // maneira terminfo
  // tputs ( cursor_normal, 1, putchar );
  // tputs ( exit_attribute_mode, 1, putchar );
  delwin ( pad );
  curs_set ( 1 );  // restore cursor
  endwin ();
}

// limpa a tela, podendo tambem limpar o buffer do scroll se disponivel
// Obs: não alterar ordem entre limpar a tela e limpar scroll
// void
// clear_cmd ( )
// {
//   // limpa a tela modo terminfo
//   // tputs ( clear_screen, lines > 0 ? lines : 1, putchar );
//   //
//   // // se recurso para limpar scroll estiver disponivel
//   // if ( E3 )
//   //   tputs ( E3, lines > 0 ? lines : 1, putchar );
// }
