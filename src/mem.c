
#include "code16.h"

#define MEM_C
#include "mem.h"
#include "console.h"
#include "adhd.h"

extern uint8_t* heap;

uint8_t g_mheap[MEM_HEAP_SIZE];
uint8_t* g_mheap_top = g_mheap;
int g_mem_pid = 0;

int mused() {
   return (int)(g_mheap_top - g_mheap);
}

/* Zero a given block of memory. */
void mzero( void* dest, int sz ) {
   int i = 0;
   for( i = 0 ; sz > i ; i++ ) {
      ((uint8_t*)dest)[i] = '\0';
   }
}

#if defined( MPRINT )
#include <stdio.h>
void mprint() {
   int i = 0;

   for( i = 0 ; MEM_HEAP_SIZE > i ; i++ ) {
      if( 0 == i % 20 ) {
         tprintf( "\n" );
      }
      if( i == g_mheap_top ) {
         tprintf( "** " );
      } else {
         /* TODO: Implement hex tprintf. */
         tprintf( "%2X ", g_mheap[i] );
      }
   }
   tprintf( "\n" );
}
#endif /* MPRINT */

void mshift( void* start, MEMLEN_T offset ) {
   MEMLEN_T i = 0;
   uint8_t* iter = NULL;

   if( 0 < offset ) {
      /* Shifting memory contents outwards. */
      for(
         iter = (uint8_t*)(g_mheap + MEM_HEAP_SIZE);
         iter >= (uint8_t*)start;
         iter--
      ) {
         *((uint8_t*)(iter + offset)) = *iter;
         *iter = 0;
      }
      g_mheap_top += offset;
   } else if( 0 > offset ) {
      /* Shifting memory contents inwards. */
      for(
         iter = (uint8_t*)start;
         (uint8_t*)(g_mheap + MEM_HEAP_SIZE) > (iter - offset);
         iter++
      ) {
         *iter = *((uint8_t*)(iter - offset));
         *((uint8_t*)(iter - offset)) = 0;
      }
      /* Offset is negative, so we're still adding it to subtract. */
      g_mheap_top += offset;
   }
}

static struct mvar* mresize(
   struct mvar* var, MEMLEN_T start, MEMLEN_T sz
) {
   MEMLEN_T size_diff = 0;

   /* Only bother resizing if a size was provided. */
   size_diff = sz - var->sz;
   if( 0 < size_diff && g_mheap_top + size_diff > MEM_HEAP_SIZE ) {
      /* Not enough heap space! */
      return NULL;
   }

   if( 0 < size_diff ) {
      mshift( start, size_diff ); 
      var = (struct mvar*)&(g_mheap[start + size_diff]);
   }

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
#ifdef __GNUC__
   mzero( &(out->data), sz );
#endif /* __GNUC__ */

   /* Advance the heap top. */
   g_mheap_top += sizeof( struct mvar ) + sz;
 
   return out;
}

void* malloc( size_t sz ) {
   MEMLEN_T mheap_addr_iter = 0;
   struct mvar* var = NULL;

   assert( 0 < mid );

   mheap_addr_iter = mget_pos( g_mem_pid, mid );
   if( 0 > mheap_addr_iter ) {
      if( MGET_NO_CREATE == sz ) {
         return NULL;
      }
      var = mcreate( sz );
   } else if( 0 < sz ) {
      var = (struct mvar*)&(g_mheap[mheap_addr_iter]);
      if( sz > var->sz ) {
         var = mresize( var, mheap_addr_iter, sz );
      }
   }

   assert( g_mheap_top + sizeof( struct mvar ) + sz < MEM_HEAP_SIZE );
  
   /* Make sure create/resize were successful. */
   if( NULL == var ) {
      return NULL;
   }

   /* Fill out the header. */
   var->pid = pid;
   var->mid = mid;
   if( 0 < sz ) {
      var->sz = sz;
   }

   return var;
}

void free( void* addr ) {
   struct mvar* var = NULL;
   MEMLEN_T sz = 0;


   var = (struct mvar*)&(g_mheap[mheap_addr_iter]);
   sz = var->sz + sizeof( struct mvar );

   mshift( mheap_addr_iter, -1 * sz );
}

