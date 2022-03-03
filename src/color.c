
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

// colors foreground_background
#define WHITE_DEFAULT 1
#define WHITE_YELLOW 2
#define WHITE_BLUE 3
#define WHITE_MAGENTA 4
#define BLACK_GREEN 5
#define BLACK_CYAN 6
#define BLACK_BLUE 7
#define BLUE_DEFAULT 8
#define CYAN_DEFAULT 9
#define CYAN_BLACK 10
#define BLUE_BLACK 11

enum schemes
{
  DEFAULT,
  ALTERNATIVE1,
  MONO,
  TOT_SCHEMES
};

static const int color_schemes[TOT_SCHEMES][TOT_ELEMENTS] = {
        [DEFAULT] =
                {
                        [RESET] = A_NORMAL | COLOR_PAIR ( WHITE_DEFAULT ),
                        [HEADER] = COLOR_PAIR ( BLACK_GREEN ),
                        [SELECTED_H] = COLOR_PAIR ( BLACK_CYAN ),
                        [SELECTED_L] = COLOR_PAIR ( CYAN_BLACK ),
                        [PATH_PROG] = COLOR_PAIR ( CYAN_DEFAULT ),
                        [NAME_PROG] = A_BOLD | COLOR_PAIR ( CYAN_DEFAULT ),
                        [PARAM_PROG] = A_DIM | COLOR_PAIR ( CYAN_DEFAULT ),
                        [CONECTIONS] = A_DIM | COLOR_PAIR ( WHITE_DEFAULT ),
                        [TREE] = COLOR_PAIR ( CYAN_DEFAULT ),
                        [RESUME] = A_DIM | COLOR_PAIR ( WHITE_DEFAULT ),
                        [RESUME_VALUE] = COLOR_PAIR ( CYAN_DEFAULT ),
                },
        [ALTERNATIVE1] =
                {
                        [RESET] = A_NORMAL | COLOR_PAIR ( WHITE_DEFAULT ),
                        [HEADER] = COLOR_PAIR ( BLACK_BLUE ),
                        [SELECTED_H] = COLOR_PAIR ( WHITE_BLUE ),
                        [SELECTED_L] = COLOR_PAIR ( BLUE_BLACK ),
                        [PATH_PROG] = COLOR_PAIR ( BLUE_DEFAULT ),
                        [NAME_PROG] = A_BOLD | COLOR_PAIR ( BLUE_DEFAULT ),
                        [PARAM_PROG] = A_DIM | COLOR_PAIR ( BLUE_DEFAULT ),
                        [CONECTIONS] = A_DIM | COLOR_PAIR ( WHITE_DEFAULT ),
                        [TREE] = A_DIM | COLOR_PAIR ( BLUE_DEFAULT ),
                        [RESUME] = A_DIM | COLOR_PAIR ( WHITE_DEFAULT ),
                        [RESUME_VALUE] = COLOR_PAIR ( BLUE_DEFAULT ),
                },
        [MONO] = {
                [RESET] = A_NORMAL,
                [HEADER] = A_REVERSE,
                [SELECTED_H] = A_DIM | A_REVERSE,
                [SELECTED_L] = A_DIM | A_REVERSE,
                [PATH_PROG] = A_DIM,
                [NAME_PROG] = A_BOLD,
                [PARAM_PROG] = A_DIM,
                [CONECTIONS] = A_DIM,
                [TREE] = A_DIM,
                [RESUME] = A_DIM,
                [RESUME_VALUE] = A_NORMAL,
        }};

static void
pairs_init ( void )
{
  init_pair ( CYAN_DEFAULT, COLOR_CYAN, -1 );  // color tree, name program
  init_pair ( BLACK_GREEN, COLOR_BLACK, COLOR_GREEN );  // color header
  init_pair ( BLACK_CYAN, COLOR_BLACK, COLOR_CYAN );
  init_pair ( WHITE_DEFAULT, COLOR_WHITE, -1 );
  init_pair ( WHITE_BLUE, COLOR_WHITE, COLOR_BLUE );
  init_pair ( WHITE_MAGENTA, COLOR_WHITE, COLOR_MAGENTA );
  init_pair ( BLUE_DEFAULT, COLOR_BLUE, -1 );
  init_pair ( WHITE_YELLOW, COLOR_WHITE, COLOR_YELLOW );
  init_pair ( BLACK_BLUE, COLOR_BLACK, COLOR_BLUE );
  init_pair ( BLUE_BLACK, COLOR_BLUE, COLOR_BLACK );
  init_pair ( CYAN_BLACK, COLOR_CYAN, COLOR_BLACK );
}

int *
get_color_scheme ( const int color_scheme )
{
  if ( !has_colors () )
    return ( int * ) color_schemes[MONO];

  start_color ();

  if ( use_default_colors () == ERR )
    assume_default_colors ( COLOR_WHITE, COLOR_BLACK );

  pairs_init ();

  return ( int * ) color_schemes[color_scheme];
}
