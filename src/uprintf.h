
#ifndef UPRINTF_H
#define UPRINTF_H

#ifdef MSP430

#define elix_dprintf( lvl, ... )
#define elix_eprintf( ... )

#elif defined( DEBUG_ELIX_CONSOLE )

#define elix_dprintf( lvl, ... ) \
   if( lvl >= VM_DEBUG_THRESHOLD ) { \
      tprintf( "(%d) " __FILE__ ": %d: ", lvl, __LINE__ ); \
      tprintf( __VA_ARGS__ ); \
      tprintf( "\n" ); \
   }

#define elix_eprintf( ... ) \
   tprintf( "(E) " __FILE__ ": %d: ", __LINE__ ); \
   tprintf( __VA_ARGS__ ); \
   tprintf( "\n" ); \

#else

#include <stdio.h>

#define elix_dprintf( lvl, ... ) \
   if( lvl >= VM_DEBUG_THRESHOLD ) { \
      printf( "(%d) " __FILE__ ": %d: ", lvl, __LINE__ ); \
      printf( __VA_ARGS__ ); \
      printf( "\n" ); \
   }

#define elix_eprintf( ... ) \
   printf( "(E) " __FILE__ ": %d: ", __LINE__ ); \
   printf( __VA_ARGS__ ); \
   printf( "\n" ); \

#endif /* DEBUG_ELIX_CONSOLE */

#endif /* !UPRINTF_H */

