
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

#ifndef HUMAN_READABLE_H
#define HUMAN_READABLE_H

#include <stdbool.h>  // bool type
#include <stdint.h>   // uint*_t type
#include <stdlib.h>   // size_t type

bool
human_readable ( char *buffer, const size_t len_buff, const uint64_t bytes );

#endif  // HUMAN_READABLE_H
