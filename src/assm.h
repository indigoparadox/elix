
#ifndef ASSM_H
#define ASSM_H

#define STATE_NONE      0
#define STATE_SECTION   1
/*! \brief Looking for parameters to previous op. */
#define STATE_PARAMS    2
/*! \brief Inside of a char literal. */
#define STATE_CHAR      3
/*! \brief Inside of a string literal. */
#define STATE_STRING    4
/**
 * \brief Inside of an integer immediate.
 *
 * 
 */
#define STATE_NUM       5
/**
 * \brief Inside of a comment.
 *
 * Will end when newline is detected.
 */
#define STATE_COMMENT   6
/**
 * \brief Inside of an alloc (memory block index).
 */
#define STATE_ALLOC     7
#define STATE_LABEL     8
#define STATE_SYSC      9

#ifdef ASSM_C

const char* gc_assm_states[] = {
   "none",
   "section",
   "params",
   "char",
   "string",
   "num",
   "comment",
   "alloc",
   "label",
   "sysc"
};

#endif /* ASSM_C */

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

