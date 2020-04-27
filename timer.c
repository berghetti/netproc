
#include <time.h>
#include <string.h>
#include <errno.h>
#include "m_error.h"

#define NSTOS 1000000000.0  // convert nanoseconds to seconds

inline static void get_time(struct timespec *);

// static struct timespec init_time = {0};
// static struct timespec end_time = {0};


// float
// timer(void)
// {
//   float dif = 0.0;
//
//   get_time(&end_time);
//
//   dif = end_time.tv_sec - init_time.tv_sec;
//   dif += (end_time.tv_nsec - init_time.tv_nsec) / NSTOS;
//
//   return dif;
// }
//
//
// void
// init_timer(void)
// {
//   get_time(&init_time);
// }

// void
// restart_timer(void)
// {
//   init_time = end_time;
// }


float
start_timer(void)
{
  struct timespec time = {0};
  get_time(&time);

  return time.tv_sec + (time.tv_nsec / NSTOS);
}

float
timer(float old_time)
{
  float dif = 0.0;
  struct timespec new_time = {0};

  get_time(&new_time);

  dif = new_time.tv_sec + (new_time.tv_nsec / NSTOS);
  dif = dif - old_time;

  return dif;
}





inline static void
get_time(struct timespec *buff_time)
{
  if ( clock_gettime(CLOCK_MONOTONIC, buff_time) == -1 )
    fatal_error("clock_gettime: %s", strerror(errno));
}
