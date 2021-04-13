
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
#include <term.h>    // setupterm, tputs, tigetstr
#include <unistd.h>  // STDOUT_FILENO
#include <ncurses.h>

#include "config.h"
#include "color.h"
#include "pid.h"
#include "show.h"
#include "m_error.h"

WINDOW *pad;

// carrega informações do terminal a associa a stdout
bool
setup_terminal ( void )
{
  // ainda necessario para exibir correr em mensagens de erro
  int err;
  if ( setupterm ( NULL, STDOUT_FILENO, &err ) == -1 )
    {
      switch ( err )
        {
          case 1:
            fprintf ( stderr, "%s\n", "terminal is hardcopy" );
            break;
          case -1:
            fprintf ( stderr, "%s\n", "no terminfo database" );
            break;
          case 0:
            fprintf ( stderr,
                      "%s\n",
                      "unknown terminal, check environment variable TERM" );
        }
      return false;
    }

  return true;
}

bool
setup_ui ( struct config_op *co )
{
  initscr ();
  cbreak ();  // disable buffering to get keypad
  noecho ();

  if ( !( pad = newpad ( LINES_PAD, COLS_PAD ) ) )
    {
      ERROR_DEBUG ( "%s", strerror ( errno ) );
      return false;
    }

  nodelay ( pad, TRUE );  // no gelay getch()
  keypad ( pad, TRUE );   // get arrow key
  scrollok ( pad, TRUE );
  curs_set ( 0 );  // cursor invisible

  co->color_scheme = define_color_scheme ();

  co->max_digits_pid = get_max_digits_pid ();

  return true;
}

void
restore_terminal ( void )
{
  delwin ( pad );
  curs_set ( 1 );  // restore cursor
  endwin ();
}
