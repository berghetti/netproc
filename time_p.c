#include <time_p.h>


float diff(struct timespec *init, struct timespec *end)
{
  float a;
  a = end->tv_sec - init->tv_sec;
  a += (end->tv_nsec - init->tv_nsec) / NSTOS;
  return a;
}
