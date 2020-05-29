#ifndef STR_H
#define STR_H

#include <stdlib.h> // type size_t

// retorna o tamanho da string até null byte ou espaço
// oque ocorrer primeiro
size_t
strlen_space ( const char *string );

// retorna a posição do ultimo caracter pesquisado,
// ou -1 caso não encontre
int
find_last_char ( const char *str, const char ch, size_t len );

#endif // STR_H
