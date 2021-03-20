
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

#include <ncurses.h>
#include "color.h"

static int color_schemes[TOT_SCHEMES][TOT_ELEMENTS] = {
        [DEFAULT] =
                {
                        [RESET] = A_NORMAL | COLOR_PAIR ( 4 ),
                        [HEADER] = COLOR_PAIR ( 2 ),
                        [SELECTED_H] = COLOR_PAIR ( 3 ),
                        [SELECTED_L] = COLOR_PAIR ( 3 ),
                        [NAME_PROG] = COLOR_PAIR ( 1 ),
                        [NAME_PROG_BOLD] = A_BOLD | COLOR_PAIR ( 1 ),
                        [CONECTIONS] = A_DIM | COLOR_PAIR ( 4 ),
                        [TREE] = COLOR_PAIR ( 1 ),
                        [RESUME] = A_DIM | COLOR_PAIR ( 4 ),
                        [RESUME_VALUE] = COLOR_PAIR ( 1 ),
                },
        [MONO] = {
                [RESET] = A_NORMAL,
                [HEADER] = A_BOLD,
                [SELECTED_H] = A_DIM,
                [SELECTED_L] = A_BOLD,
                [NAME_PROG] = A_DIM,
                [NAME_PROG_BOLD] = A_BOLD,
                [CONECTIONS] = A_DIM,
                [TREE] = A_BOLD,
                [RESUME] = A_DIM,
                [RESUME_VALUE] = A_BOLD,
        }};

// const int *color_scheme;

int *
define_color_scheme ( void )
{
  if ( !has_colors () )
    return color_schemes[MONO];
  else
    {
      start_color ();

      if ( use_default_colors () == ERR )
        assume_default_colors ( COLOR_WHITE, COLOR_BLACK );

      init_pair ( 1, COLOR_CYAN, -1 );            // color tree, name program
      init_pair ( 2, COLOR_BLACK, COLOR_GREEN );  // color header
      init_pair ( 3, COLOR_BLACK, COLOR_CYAN );  // line selected, column sorted
      init_pair ( 4, COLOR_WHITE, -1 );

      // color_scheme = color_schemes[DEFAULT];
      return color_schemes[DEFAULT];
    }
}
