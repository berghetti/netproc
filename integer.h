#ifndef INTEGER_H
#define INTEGER_H

// determina se um valor float é inteiro
// com base nas casas decimais (precision) fornecida
// @float n, numero a ter testado
// @int precision, casas decimais para testar
// @round, 1 arredonda, 0 não
int
is_integer ( const float n, const int precision, const int round );

#endif  // INTEGER_H
