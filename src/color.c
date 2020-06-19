
#include <ncurses.h>
#include "color.h"

// enum schemes
// {
//   DEFAULT,
//   MONO,
//   TOT_SCHEMES
// };
//
// enum elements_in_schemes
// {
//   HEADER,
//   SELECTED,
//   NAME_PROG,
//   TOT_ELEMENTS
// };

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
                        [TREE] = A_BOLD | COLOR_PAIR ( 1 ),
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
        }};

// const int *color_scheme;

int *
define_color_scheme ( void )
{
  if ( !has_colors () )
    // color_scheme = color_schemes[MONO];
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
