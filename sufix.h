#ifndef SUFIX_H
#define SUFIX_H

#define BASE_IEC 1024  // default
#define BASE_SI 1000

#define LEN_ARR_SUFIX 5

// n = BASE_IEC ? 1 /1024 : 1/1000
#define INVERSE_BASE( n ) ( ( n ) == BASE_IEC ) ? 9.76562E-4 : 1E-3

extern int chosen_base;
extern const char *const *sufix;

void
define_sufix ( void );

#endif  // SUFIX_H
