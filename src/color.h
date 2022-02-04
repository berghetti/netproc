
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

#ifndef COLOR_H
#define COLOR_H

#include "config.h"  // struct config_op

enum elements_in_schemes
{
  RESET,
  HEADER,
  SELECTED_H,
  SELECTED_L,
  PATH_PROG,
  NAME_PROG,
  PARAM_PROG,
  CONECTIONS,
  TREE,
  RESUME,
  RESUME_VALUE,
  TOT_ELEMENTS
};

int *
get_color_scheme ( const int color_scheme );

#endif  // COLOR_H
