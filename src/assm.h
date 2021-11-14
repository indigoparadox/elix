
#ifndef ASSM_H
#define ASSM_H

#ifndef DEBUG_THRESHOLD
#define DEBUG_THRESHOLD 1
#endif /* !DEBUG_THRESHOLD */

#define assm_dprintf( lvl, ... ) \
   if( lvl >= DEBUG_THRESHOLD ) { \
      printf( "(%d) " __FILE__ ": %d: ", lvl, __LINE__ ); \
      printf( __VA_ARGS__ ); \
      printf( "\n" ); \
      fflush( stdout ); \
   }

#define assm_eprintf( ... ) \
   fprintf( stderr, "(E) " __FILE__ ": %d: ", __LINE__ ); \
   fprintf( stderr, __VA_ARGS__ ); \
   fprintf( stderr, "\n" ); \
   fflush( stderr ); \

#endif /* ASSM_H */

