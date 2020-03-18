#ifndef TIME_P_H
#define TIME_P_H

#include <time.h>
#include "time_p.h"

#define NSTOS 1000000000.0  // convert nanoseconds for seconds


// retorna a diferença de tempo de init e end
float diff(struct timespec *init, struct timespec *end);



#endif //TIME_P_H
