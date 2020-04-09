
#include "timer.h"

#define NSTOS 1000000000.0  // convert nanoseconds for seconds

static struct timespec init_time, end_time;


float
timer(void)
{
  float dif = 0.0;

  if (clock_gettime(CLOCK_MONOTONIC, &end_time) == -1 )
    perror("clock_gettime");

  dif = end_time.tv_sec - init_time.tv_sec;
  dif += (end_time.tv_nsec - init_time.tv_nsec) / NSTOS;
  
  return dif;
}


void
init_timer(void)
{
  if (clock_gettime(CLOCK_MONOTONIC, &init_time) == -1)
    {
      perror("clock_gettime");
      exit(EXIT_FAILURE);
    }
}

void
restart_timer(void)
{
  init_time = end_time;
}
