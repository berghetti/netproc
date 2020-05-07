
// determina se um valor float é inteiro
// com base nas casas decimais (precision) fornecida
// @float n, numero a ter testado
// @int precision, casas decimais para testar
// @round, 1 arredonda, 0 não
int
is_integer ( float n, int precision, int round )
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
