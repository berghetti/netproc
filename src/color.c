
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

int color_schemes[TOT_SCHEMES][TOT_ELEMENTS] = {
  [DEFAULT] = {
    [RESET] = A_NORMAL | COLOR_PAIR(6),
    [HEADER] = COLOR_PAIR ( 2 ),
    [SELECTED] = COLOR_PAIR ( 4 ),
    [NAME_PROG] = COLOR_PAIR ( 1 ),
    [NAME_PROG_BOLD] = A_BOLD | COLOR_PAIR ( 1 ),
  },
  [MONO] = {
    [HEADER] = COLOR_PAIR(5),
    [SELECTED] = COLOR_PAIR(5),
    [NAME_PROG] = COLOR_PAIR(6),
    [NAME_PROG_BOLD] = A_BOLD | COLOR_PAIR ( 6 ),
  }
};

int *color_scheme;

void define_color_scheme(void)
{
  start_color ();
  use_default_colors ();

  init_pair ( 1, COLOR_CYAN, -1 );            // color tree, name program
  init_pair ( 2, COLOR_BLACK, COLOR_GREEN );  // color header
  init_pair ( 3, -1, COLOR_BLACK );
  init_pair ( 4, COLOR_BLACK, COLOR_CYAN );  // line selected, column sorted

  //mono
  init_pair ( 5, COLOR_BLACK, COLOR_WHITE);
  init_pair ( 6, COLOR_WHITE, -1);

  color_scheme = color_schemes[DEFAULT];
  // init_pair ( 7, COLOR_BLACK, COLOR_WHITE);
  // init_pair ( 8, COLOR_BLACK, COLOR_WHITE);
}
