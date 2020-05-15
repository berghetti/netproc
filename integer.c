
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


// determina se um valor float é inteiro
// com base nas casas decimais (precision) fornecida
// @float n, numero a ter testado
// @int precision, casas decimais para testar
// @round, 1 arredonda, 0 não
int
is_integer ( const float n, const int precision, const int round )
{
  // pow(10, precision)
  int mult = 1;
  for ( int i = 0; i < precision; i++ )
    mult *= 10;

  // example n == 1,005859 precision == 2
  // adiciona 0,005 em n, n == 1,010859 (caso arredondar)
  // multiplica n por 100, n == 101,0859
  // pega a parte inteira, n == 101
  // divide por 100, n == 1,01
  // return false, pois 1,01 != 1
  // caso não tivesse arredondado, retornaria true

  float temp;

  if ( round )
    temp = ( float ) ( ( int ) ( ( n + ( ( float ) 5 / ( mult * 10 ) ) ) *
                                 mult ) ) /
           mult;
  else
    temp = ( float ) ( ( int ) ( ( float ) n * mult ) ) / mult;

  return ( ( float ) temp == ( int ) n );
}
