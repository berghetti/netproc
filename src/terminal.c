
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
#include "m_error.h"

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
