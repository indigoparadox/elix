
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
void mshift( MEMLEN_T start, SMEMLEN_T offset );
SMEMLEN_T mget_pos( int pid, int mid );
#endif /* CHECK */

#if defined( MPRINT )
void mprint();
#endif /* MPRINT */

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

void mzero( void* dest, MEMLEN_T sz );
SMEMLEN_T mcopy( void* dest, const void* src, MEMLEN_T sz );
SMEMLEN_T mcompare( const void* c1, const void* c2, MEMLEN_T sz );
void mfree_all( TASK_PID pid );
void mfree( TASK_PID pid, MEM_ID mid );

/*! @} */

#endif /* MEM_H */

