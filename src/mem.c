
#include "mem.h"

#include <stddef.h>

struct mvar {
   int pid;
   int mid;
   int size;
};

static uint8_t g_mheap[MEM_HEAP_SIZE];
static int g_mheap_top = 0;

void minit() {
   mzero( g_mheap, MEM_HEAP_SIZE );
}

void mzero( void* dest, int sz ) {
   int i = 0;
   for( i = 0 ; sz > i ; i++ ) {
      ((uint8_t*)dest)[i] = '\0';
   }
}

int mcopy( void* dest, const void* src, int sz ) {
   int i = 0;
   for( i = 0 ; sz > i ; i++ ) {
      ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
   }
   return i;
}

int mcompare( const void* c1, const void* c2, int sz ) {
   int i = 0;
   for( i = 0 ; sz > i ; i++ ) {
      if( ((uint8_t*)c1)[i] != ((uint8_t*)c2)[i] ) {
         return 1;
      }
   }
   return 0;
}

int mstrlen( const char* str ) {
   int i = 0;
   while( '\0' != str[i] ) { i++; }
   return i;
}

/*
struct mstring* mfromcstr( char* str ) {
   char* c = str;
   int len = 0;
   while( '\0' != *c ) {
      c++;
      len++;
   }
}
*/

void mset( int pid, int mid, void* ptr, int len ) {
   struct mvar* var = NULL;

   if( g_mheap_top + sizeof( struct mvar ) +  len > MEM_HEAP_SIZE ) {
      /* Not enough heap available! */
      return;
   }

   /* Create the variable. */
   var = (struct mvar*)&(g_mheap[g_mheap_top]);
   var->pid = pid;
   var->mid = mid;
   var->size = len;
   mcopy(
      &(g_mheap[g_mheap_top + sizeof( struct mvar )]), /* Past the header. */
      ptr, len );

   /* Advance the heap top. */
   g_mheap_top += len;
}

/* Copy the result into a buffer provided by the process, for security 
 * checks.
 */
void mget( int pid, int mid, void* buffer, int buffer_sz ) {
   struct mvar* var_iter = (struct mvar*)g_mheap;
   int mheap_addr_iter = 0;

   if( 0 >= g_mheap_top ) {
      /* Heap is empty. */
      return;
   }

   while( pid != var_iter->pid && mid != var_iter->mid ) {
      if( mheap_addr_iter < g_mheap_top ) {
         /* Unable to find it and ran out of heap! */
         return;
      }

      /* Advance past the variable and metadara. */
      mheap_addr_iter += var_iter->size;
      mheap_addr_iter += sizeof( struct mvar );

      /* Advance the pointer to match. */
      var_iter = (struct mvar*)&(g_mheap[mheap_addr_iter]);
   }

   if( pid != var_iter->pid || mid != var_iter->mid ) {
      /* Unable to find it! */
      return;
   }

   if( buffer_sz < var_iter->size ) {
      /* Buffer too small! */
      return;
   }

   /* Return the value, and ONLY the value. */
   mheap_addr_iter += sizeof( struct mvar );
   mcopy( buffer, &(g_mheap[mheap_addr_iter]), var_iter->size );
}

