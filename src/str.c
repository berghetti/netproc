
#include <stdlib.h> // type size_t

// retorna o tamanho da string até null byte ou espaço
// oque ocorrer primeiro
size_t
strlen_space ( const char *string )
{
  size_t n = 0;
  while ( *string && *string++ != ' ' )
    n++;

  return n;
}

// return position of last occurrence of character in string
// char *str, pointer to string
// size_t len, lenght of string
// char ch, character to search
int
find_last_char ( const char *str, size_t len, const char ch )
{
  while ( len-- )
    if ( str[len] == ch )
      return len;

  // not found
  return -1;
}
