
#define MEM_C
#include "mem.h"
#include "console.h"
#include "strings.h"

#include <stddef.h>

#ifndef CHECK
static
#endif /* CHECK */
uint8_t g_mheap[MEM_HEAP_SIZE];
#ifndef CHECK
static
#endif /* CHECK */
int g_mheap_top = 0;

/* Setup the heap. */
void minit() {
   mzero( g_mheap, MEM_HEAP_SIZE );
   g_mheap_top = 0;
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

#if defined( MPRINT ) || defined( CHECK )
void mprint() {
   int i = 0;

   for( i = 0 ; MEM_HEAP_SIZE > i ; i++ ) {
      if( 0 == i % 20 ) {
         tputs( &g_str_newline );
      }
      if( i == g_mheap_top ) {
         tputs( &g_str_xx );
      } else {
         /* TODO: Implement hex tprintf. */
         //printf( "%02X ", g_mheap[i] );
         tprintf( &g_str_x, g_mheap[i] );
      }
   }
   tputs( &g_str_newline );
}
#endif /* MPRINT || CHECK */

/* Get the heap position for a variable. Used in public functions below. */
static int mget_pos( int pid, int mid ) {
   struct mvar* var_iter = (struct mvar*)g_mheap;
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
void mshift( int start, int offset ) {
   int i = 0;

   for( i = g_mheap_top ; i >= start ; i-- ) {
      g_mheap[i + offset] = g_mheap[i];
      g_mheap[i] = '\0';
   }

   g_mheap_top += offset;
}

void* mget( int pid, int mid, void* src, STRLEN_T sz ) {
   STRLEN_T mheap_addr_iter = 0;
   struct mvar* var = NULL;
   STRLEN_T size_diff = 0;

   /* Variables on the heap are separated by NULL spacers as noted below.
    * This is so that strlen() and co still work.
    */
   mheap_addr_iter = mget_pos( pid, mid );

   if( 0 <= mheap_addr_iter ) {
      /* Found, so make sure there's room on the heap to update it. */
      var = (struct mvar*)&(g_mheap[mheap_addr_iter]);

      size_diff = sz - (var->size - 1); /* -1 for extra NULL. */
      if( 0 < size_diff && g_mheap_top + size_diff > MEM_HEAP_SIZE ) {
         /* Not enough heap space! */
         /* Strip const because we'll crash if it's reffed anyway. */
         /* It's NULL, after all. */
         return (void*)&meta_null;
      }

      mshift( mheap_addr_iter, size_diff ); /* +1 to put the NULL back. */
   } else {
      /* Not found. Create it. */
      if( g_mheap_top + sizeof( struct mvar ) + sz > MEM_HEAP_SIZE ) {
         /* Not enough heap available! */
         /* Strip const because we'll crash if it's reffed anyway. */
         /* It's NULL, after all. */
         return (void*)&meta_null;
      }

      /* Move to the next free spot. */
      mheap_addr_iter = g_mheap_top;
      var = (struct mvar*)&(g_mheap[mheap_addr_iter]);

      /* Advance the heap top. */
      g_mheap_top += sizeof( struct mvar ) + sz + 1;
   }

   /* Fill out the header. */
   var->pid = pid;
   var->mid = mid;
   var->size = sz + 1; /* +1 for extra NULL spacer. */
   mheap_addr_iter += sizeof( struct mvar );

   /* Copy the provided buffer in if there is one. */
   if( NULL != src ) {
      mcopy( &(g_mheap[mheap_addr_iter]), src, sz );

      /* Set the len property only if this is a string anyway. */
      while( '\0' != var->data[var->len] && var->len < var->size ) {
         var->len++;
      }
   }

   /* Add an extra NULL at the end. */
   g_mheap[mheap_addr_iter + var->size] = '\0';

   mheap_addr_iter += sizeof( struct mvar );
   return &(g_mheap[mheap_addr_iter]);
}

