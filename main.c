#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "process.h"


int main(int argc, char **argv){

  process_t *process = NULL;
  int tot_process_act = 0;

    tot_process_act = get_process_active_con(&process);


    // printf("tot aqui %d\n", tot);


    print_process(process, tot_process_act);

    free_process(process, tot_process_act);




  return 0;
}
