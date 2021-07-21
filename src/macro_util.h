
#ifndef MACRO_UTIL_H
#define MACRO_UTIL_H

#define MAX( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )

#ifdef __GNUC__
#define UNUSED( x ) __attribute__ ( ( __unused__ ) ) x
#else
#define UNUSED( x )
#endif

#endif  // MACRO_UTIL_H
