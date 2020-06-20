#ifndef TRANSLETE_H
#define TRANSLETE_H

#include "conection.h"
#include "config.h"

char *
translate ( const conection_t *restrictcon,
            const struct config_op *restrict co );

#endif  // TRANSLETE_H
