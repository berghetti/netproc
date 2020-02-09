#include <stdio.h>
#include <stdint.h>
#include "process.h"


int main(int argc, char **argv){

  process_t *process = NULL;
  int tot = get_process_active_con(&process);

  printf("tot aqui %d\n", tot);

  if (process)
    printf("process pid %d\n",
            process[0].pid);

  int tmp;
  int f;
  for (int i = 0; i < tot; i++)
    {



        printf("process pid      - %d\n"
               "process name     - %s\n"
               "tot fds          - %d\n"
               "total conections - %d\n",
                process[i].pid, process[i].name, process[i].total_fd,
                process[i].total_conections);
        printf("inodes           - ");

        f = 0;
        tmp = process[i].total_conections;

        while(tmp--)
          printf("%d ", process[i].conection[f++].inode);


      printf("\n\n");
    }



  return 0;
}
