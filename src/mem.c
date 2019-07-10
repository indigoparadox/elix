
#include "code16.h"

#define MEM_C
#include "mem.h"
#include "adhd.h"
#include "kernel.h"

#ifdef MPRINT
#include "console.h"
#include "strings.h"
#endif /* MPRINT */

#include <stddef.h>

#ifdef DEBUG
#ifdef MEM_PRINTF_TRACE
#include <stdio.h>
#endif /* MEM_PRINTF_TRACE */
#endif /* DEBUG */

#ifdef DEBUG
#include <assert.h>
#else
#define assert( x )
#endif /* DEBUG */

extern uint8_t* heap;

#ifndef CHECK
static
#endif /* CHECK */
uint8_t g_mheap[MEM_HEAP_SIZE];
#ifndef CHECK
static
#endif /* CHECK */
MEMLEN_T g_mheap_top = 0;

#ifdef __GNUC__
__attribute__( (constructor( CTOR_PRIO_MEM )) )
#endif /* __GNUC__ */
static void minit() {
   /* Setup the heap. */
   mzero( g_mheap, MEM_HEAP_SIZE );
   g_mheap_top = 0;
}

MEMLEN_T get_mem_used() {
   return g_mheap_top;
}

/* Zero a given block of memory. */
void mzero( void* dest, int sz ) {
   int i = 0;
   for( i = 0 ; sz > i ; i++ ) {
      ((uint8_t*)dest)[i] = '\0';
   }
}

/* memcpy */
int mcopy( void* dest, const void* src, int sz ) {
   int i = 0;
   for( i = 0 ; sz > i ; i++ ) {
      ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
   }
   return i;
}

/* strncmp */
int mcompare( const void* c1, const void* c2, int sz ) {
   int i = 0;
   for( i = 0 ; sz > i ; i++ ) {
      if( ((uint8_t*)c1)[i] != ((uint8_t*)c2)[i] ) {
         return 1;
      }
   }
   return 0;
}

#if defined( MPRINT )
void mprint() {
   int i = 0;

   for( i = 0 ; MEM_HEAP_SIZE > i ; i++ ) {
      if( 0 == i % 20 ) {
         tprintf( CONSOLE_NEWLINE );
      }
      if( i == g_mheap_top ) {
         tprintf( "** " );
      } else {
         /* TODO: Implement hex tprintf. */
         tprintf( "%2X ", g_mheap[i] );
      }
   }
   tprintf( CONSOLE_NEWLINE );
}
#endif /* MPRINT */

/* Get the heap position for a variable. Used in public functions below. */
#ifndef CHECK
static
#endif /* CHECK */
int mget_pos( int pid, int mid ) {
   const struct mvar* var_iter = (struct mvar*)g_mheap;
   int mheap_addr_iter = 0;

   if( 0 >= g_mheap_top ) {
      /* Heap is empty. */
      return -1;
   }

   /* printf( "mpos_pid: %d (%d), mpos_mid: %d (%d)\n",
      pid, var_iter->pid, mid, var_iter->mid ); */

   /* Hunt until we find a var tagged with the sought pid and mid. */
   while( pid != var_iter->pid || mid != var_iter->mid ) {
      if( mheap_addr_iter >= g_mheap_top ) {
         /* Unable to find it and ran out of heap! */
         return -1;
      }

      /* Advance past the variable and metadara. */
      mheap_addr_iter += var_iter->size;
      mheap_addr_iter += sizeof( struct mvar );

      /* Advance the pointer to match. */
      var_iter = (struct mvar*)&(g_mheap[mheap_addr_iter]);
   }

   if( pid != var_iter->pid || mid != var_iter->mid ) {
      /* Unable to find it! */
      return -1;
   }

   return mheap_addr_iter;
}

#ifndef CHECK
static
#endif /* CHECK */
void mshift( MEMLEN_T start, MEMLEN_T offset ) {
   MEMLEN_T i = 0;

   for( i = g_mheap_top ; i >= start ; i-- ) {
      g_mheap[i + offset] = g_mheap[i];
      g_mheap[i] = 0;
   }

   g_mheap_top += offset;
}

static struct mvar* mresize(
   struct mvar* var, MEMLEN_T start, MEMLEN_T sz
) {
   MEMLEN_T size_diff = 0;

   /* Only bother resizing if a size was provided. */
   size_diff = sz - var->size;
   if( 0 < size_diff && g_mheap_top + size_diff > MEM_HEAP_SIZE ) {
      /* Not enough heap space! */
      return NULL;
   }

   if( 0 < size_diff ) {
      mshift( start, size_diff ); 
      var = (struct mvar*)&(g_mheap[start + size_diff]);
      var->size = sz;
   }

   assert( 0 != sz );
   assert( sz == var->size );

   return var;
}

static struct mvar* mcreate( MEMLEN_T sz ) {
   struct mvar* out = NULL;

   /* Not found. Create it. */
   if(
      0 >= sz || /* Need a sz to create! */
      g_mheap_top + sizeof( struct mvar ) + sz > MEM_HEAP_SIZE
   ) {
      /* Not enough heap available! */
      return NULL;
   }

   /* Move to the next free spot and reset convenience ptr. */
   out = (struct mvar*)&(g_mheap[g_mheap_top]);
   out->size = sz;
   mzero( &(out->data), sz );

   /* Advance the heap top. */
   g_mheap_top += sizeof( struct mvar ) + sz;
 
   return out;
}

static struct mvar* mfind( TASK_PID pid, MEM_ID mid, MEMLEN_T sz ) {
   MEMLEN_T mheap_addr_iter = 0;
   struct mvar* var = NULL;

   mheap_addr_iter = mget_pos( pid, mid );
   if( 0 > mheap_addr_iter ) {
      if( MGET_NO_CREATE == sz ) {
         return NULL;
      }
      var = mcreate( sz );
   } else {
      var = (struct mvar*)&(g_mheap[mheap_addr_iter]);
      if( sz > var->size ) {
         var = mresize( var, mheap_addr_iter, sz );
      }
   }

   return var;
}

/**
 * \brief Get a dynamic variable.
 *
 * @param sz   The size (in bytes) of the variable to allocate.
 *             Set to MGET_NO_CREATE to not allocate the variable if it is not
 *             already allocated.
 *             Set to MGET_UNSET to unset it if it is.
 */
const void* mget( TASK_PID pid, MEM_ID mid, MEMLEN_T sz ) {
   struct mvar* var = NULL;

   assert( 0 < mid );

   var = mfind( pid, mid, sz );
   if( NULL == var ) {
      return NULL;
   }

   /* Fill out the header. */
   var->pid = pid;
   var->mid = mid;
   /*if( 0 < sz && 0 == var->size ) {
      var->size = sz;
   }*/
   assert( 0 >= sz || sz == var->size );

   return &(var->data);
}

void mset( TASK_PID pid, MEM_ID mid, MEMLEN_T sz, const void* data ) {
   struct mvar* var = NULL;
   /* const uint8_t* data_bytes = (uint8_t*)data;
   uint8_t* mem_bytes = NULL; */

   var = mfind( pid, mid, sz );
   if( NULL == var ) {
      return;
   }

   if( NULL != data ) {
      mcopy( var->data, data, sz );
   }
}

void meditprop(
   TASK_PID pid, MEM_ID mid, MEMLEN_T offset, MEMLEN_T sz, void* val
) {
   struct mvar* var = NULL;
   uint8_t* bytes = NULL;
   
   var = mfind( pid, mid, MGET_NO_CREATE );
   if( NULL == var ) {
      return;
   }

   bytes = &(var->data[offset]);
   mcopy( bytes, val, sz );
   
}

void mgetprop(
   TASK_PID pid, MEM_ID mid, MEMLEN_T offset, MEMLEN_T sz, void* dest
) {
   struct mvar* var = NULL;
   
   var = mfind( pid, mid, MGET_NO_CREATE );
   if( NULL == var ) {
      return;
   }

   mcopy( dest, &(var->data[offset]), sz );
   
}

int mincr( TASK_PID pid, MEM_ID mid ) {
   struct mvar* var = NULL;
   int* iptr = NULL;

   var = mfind( pid, mid, sizeof( int ) );
   if( NULL == var ) {
      return 0;
   }

   assert( var->size == sizeof( int ) );

   iptr = (int*)(var->data);

   (*iptr)++;

   return *iptr;
}

