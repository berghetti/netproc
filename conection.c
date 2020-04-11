
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "conection.h"
#include "m_error.h"

// le o arquivo onde fica salva as conexoes '/proc/net/tcp',
// recebe o local do arquivo, um buffer para armazenar
// dados da conexão e o tamanho do buffer,
// retorna a quantidade de registros encontrada
// ou -1 em caso de erro
int
get_info_conections(conection_t *conection,
                    const size_t lenght,
                    const char *conection_file)
{

  FILE *arq = NULL;

  if (!(arq = fopen(conection_file, "r")))
    return -1;


  char *line = NULL;
  size_t len = 0;

  // ignore header in first line
  if ((getline(&line, &len, arq)) == -1)
    {
      free(line);
      line = NULL;
      fclose(arq);
      return -1;
    }


  uint32_t count = 0;
  char local_addr[64], rem_addr[64] = {0};

  unsigned int matches, local_port, rem_port; // local_addr, rem_addr;
  unsigned long int inode;

  while ( (getline(&line, &len, arq)) != -1 && (count < lenght) )
    {

// sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
//  0: 0100007F:7850 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 19860 1 00000000d8098b1f 100 0 0 10 0
//  1: 0100007F:78B4 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 19859 1 00000000bb21a0eb 100 0 0 10 0
//  2: 9902A8C0:86CA BCC0D9AC:146C 01 00000000:00000000 02:00000CB1 00000000  1000        0 62034 2 0000000028d298f1 36 4 0 10 -1
//  3: 9902A8C0:9A26 AB2B4E68:01BB 01 00000000:00000000 02:00000178 00000000  1000        1 2612130 2 0000000014dc9e6e 52 4 10 10 -1
//  4: 9902A8C0:93A6 1A71528C:01BB 01 00000000:00000000 02:00000D3E 00000000  1000        0 2277146 2 000000001c1e11b6 46 4 27 10 -1
//  5: 9902A8C0:C582 7CFD1EC0:01BB 01 00000000:00000000 02:000001E2 00000000  1000        0 2437598 2 00000000bd8b447a 46 4 27 10 -1

      matches = sscanf(line, "%*d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %*X "
                       "%*X:%*X %*X:%*X %*X %*d %*d %lu %*512s\n",
                       local_addr, &local_port, rem_addr, &rem_port, &inode);

      if (matches != 5)
        return -1;


      if (strlen(local_addr) == 8) // only ipv4
        {
          // converte char para tipo inteiro
          if (1 != sscanf(local_addr, "%x", &conection[count].local_address))
            error("Error converting ip address: %s", strerror(errno));

          if (1 != sscanf(rem_addr, "%x", &conection[count].remote_address))
            error("Error converting ip address: %s", strerror(errno));

          conection[count].local_port = local_port;
          conection[count].remote_port = rem_port;
          // conection[count].con_state = con_state;
          conection[count].inode = inode;
          // conection[count].id = id;

          count++;
        }

      }

  free(line);
  line = NULL;
  fclose(arq);

  return count;
}
