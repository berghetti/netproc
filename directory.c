
#include "directory.h"

static bool is_number(const char *string);

// recebe o nome de um diretorio, um buffer para armazenar
// os nomes dos diretorios que são numericos e um tamanho maximo do buffer.
// retorna o total de diretorios encontrados, -1 em caso de falha.
int
get_numeric_directory(uint32_t *buffer,
                      const size_t lenght,
                      const char *path_dir)
{
  DIR *dir;

  if ((dir = opendir(path_dir)) == NULL)
    return -1;

  struct dirent *directory = NULL;
  uint32_t count = 0;

  while ( (directory = readdir(dir)) && count < lenght )
      if (is_number(directory->d_name))
          buffer[count++] = atoi(directory->d_name);


  closedir(dir);

  return count;
}

static bool is_number(const char *string)
{
  while (*string)
      if (! isdigit(*string++))
        return false;

  return true;
}
