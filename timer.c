
#include <time.h>
#include <string.h>
#include <errno.h>
#include "m_error.h"

#define NSTOS 1000000000.0  // convert nanoseconds for seconds

inline static void get_time(struct timespec *);

static struct timespec init_time, end_time;


float
timer(void)
{
  float dif = 0.0;

  get_time(&end_time);

  dif = end_time.tv_sec - init_time.tv_sec;
  dif += (end_time.tv_nsec - init_time.tv_nsec) / NSTOS;

  return dif;
}


void
init_timer(void)
{
  get_time(&init_time);
}

void
restart_timer(void)
{
  init_time = end_time;
}

inline static void
get_time(struct timespec *buff_time)
{
  if (clock_gettime(CLOCK_MONOTONIC, buff_time) == -1 )
    fatal_error("fault clock_gettime: %s", strerror(errno));
}
