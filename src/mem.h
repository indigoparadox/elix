
#ifndef MEM_H
#define MEM_H

/*! \file mem.h */

#include <stdint.h>
#include <stddef.h>

#include "platform.h"

typedef int16_t MEMLEN_T;
typedef uint16_t BITFIELD;
typedef uint8_t MEM_ID;
typedef uint8_t MEM_TYPE;

#define ADHD_DEF_ONLY
#include "adhd.h"

#define MEM_NULL 0
#define MEM_INT 1
#define MEM_UINT8 2
#define MEM_CHAR 3
#define MEM_PTR 4

#define MGET_UNSET      -1
#define MGET_NO_CREATE  0

#include "alpha.h"
union mvalue {
   UTOA_T d;
   char c;
   uint8_t x;
   void* p;
   char* s;
};

struct mvar {
   uint8_t pid;
   MEM_ID mid;
   MEMLEN_T size; /* Allocated. */
   uint8_t data[];
} __attribute__((packed));

#ifndef MEM_HEAP_SIZE
#define MEM_HEAP_SIZE 10240
#endif /* !MEM_HEAP_SIZE */

#ifdef CHECK
void mshift( MEMLEN_T start, MEMLEN_T offset );
#endif /* CHECK */

#if defined( MPRINT )
void mprint();
#endif /* MPRINT */

#ifdef CHECK
int mget_pos( int pid, int mid );
#endif /* CHECK */

MEMLEN_T get_mem_used();

/**
 * \brief Get a dynamic variable.
 *
 * @param sz   The size (in bytes) of the variable to allocate.
 *             Set to MGET_NO_CREATE to not allocate the variable if it is not
 *             already allocated.
 *             Set to MGET_UNSET to unset it if it is.
 */
const void* mget( TASK_PID pid, MEM_ID mid, MEMLEN_T sz );

void mgetprop(
   TASK_PID pid, MEM_ID mid, MEMLEN_T offset, MEMLEN_T sz, void* dest );
void mset( TASK_PID pid, MEM_ID mid, MEMLEN_T sz, const void* data );
void mzero( void* dest, int sz );
int mcopy( void* dest, const void* src, int sz );
int mcompare( const void* c1, const void* c2, int sz );
int mincr( TASK_PID pid, MEM_ID mid );
void meditprop(
   TASK_PID pid, MEM_ID mid, MEMLEN_T offset, MEMLEN_T sz, void* val
);

#ifdef MEM_C
const long meta_null = 0;
#else
extern const long meta_null;
#endif /* MEM_C */

#endif /* MEM_H */

