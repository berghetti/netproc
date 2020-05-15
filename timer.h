
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

#ifndef M_TIME_H
#define M_TIME_H

// para facilitar a leitura do codigo
#define restart_timer start_timer

// retorna a diferença entre o tempo atual em segundos
// e o valor em segundos passado por parametro
double
timer ( const float old_time );

// retorna a tempo atual em segundos
double
start_timer ( void );

#endif  // MTIME_H
