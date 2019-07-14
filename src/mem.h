
#ifndef MEM_H
#define MEM_H

/*! \file mem.h */

#include "kernel.h"

#define MGET_UNSET      -1
#define MGET_NO_CREATE  0

union mvalue {
   UTOA_T d;
   char c;
   uint8_t x;
   void* p;
   char* s;
};

#ifndef MEM_HEAP_SIZE
#define MEM_HEAP_SIZE 300
#endif /* MEM_HEAP_SIZE */

void minit();

#ifdef CHECK
void mshift( MEMLEN_T start, MEMLEN_T offset );
int mget_pos( int pid, int mid );
#endif /* CHECK */

#if defined( MPRINT ) || defined( CHECK )
void mprint();
#endif /* MPRINT || CHECK */

//void minit();
MEMLEN_T get_mem_used();

/**
 * \brief Get or set a dynamic variable.
 *
 * @param sz   The size (in bytes) of the variable to allocate.
 *             Set to MGET_NO_CREATE to not allocate the variable if it is not
 *             already allocated.
 *             Set to MGET_UNSET to unset it if it is.
 */
void* mget( TASK_PID pid, MEM_ID mid, MEMLEN_T sz );

struct mvar* mget_meta( TASK_PID pid, MEM_ID mid, MEMLEN_T sz );

/* \addtogroup ptr_tools Pointer Tools 
 *  @{
 */

void mzero( void* dest, int sz );
int mcopy( void* dest, const void* src, int sz );
int mcompare( const void* c1, const void* c2, int sz );
void mfree( TASK_PID pid, MEM_ID mid );

/*! @} */

#endif /* MEM_H */

