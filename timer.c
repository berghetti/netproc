
#include <time.h>
#include <string.h>
#include <errno.h>
#include "m_error.h"

// #define NSTOS 1000000000.0  // convert nanoseconds to seconds

// multiply nanoseconds for this const convert nanoseconds to seconds
#define NSTOS 1E-9


inline static void get_time(struct timespec *);


float
start_timer(void)
{
  struct timespec time = {0};
  get_time(&time);

  // return time.tv_sec + (time.tv_nsec / NSTOS);
  return (float) time.tv_sec + (time.tv_nsec * NSTOS) ;

}

float
timer(const float old_time)
{
  float dif = 0.0;
  struct timespec new_time = {0};

  get_time(&new_time);

  // dif = new_time.tv_sec + (new_time.tv_nsec / NSTOS);
  dif = new_time.tv_sec + (new_time.tv_nsec * NSTOS);
  dif = dif - old_time;

  return dif;
}


inline static void
get_time(struct timespec *buff_time)
{
  if ( clock_gettime(CLOCK_MONOTONIC, buff_time) == -1 )
    fatal_error("clock_gettime: %s", strerror(errno));
}
