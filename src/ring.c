
#include "ring.h"

#include "io.h"

#ifdef DEBUG
#include <assert.h>
#else
#define assert( x )
#endif /* DEBUG */

const struct ring_buffer* ring_buffer( uint8_t pid, MEM_ID mid, MEMLEN_T sz ) {
   const struct ring_buffer* rb = NULL;

   // mset( pid, mid, sizeof( struct ring_buffer ) + sz, NULL );
   rb = mget( pid, mid, sizeof( struct ring_buffer ) + sz );
   if( 0 == rb->sz ) {
      meditprop(
         pid, mid,
         offsetof( struct ring_buffer, sz ), sizeof( MEMLEN_T ), &sz );
   }

   return rb;
}

void ring_buffer_push( uint8_t pid, MEM_ID mid, char val ) {
   MEMLEN_T start = 0, sz = 0, end = 0;

   mget( pid, mid, sizeof( struct ring_buffer ) + sz );

   mgetprop( pid, mid, offsetof( struct ring_buffer, sz ),
      sizeof( MEMLEN_T ), &sz );
   mgetprop( pid, mid, offsetof( struct ring_buffer, start ),
      sizeof( MEMLEN_T ), &start );
   mgetprop( pid, mid, offsetof( struct ring_buffer, end ),
      sizeof( MEMLEN_T ), &end );

   /* Append the val onto the end. */
   meditprop(
      pid, mid,
      offsetof( struct ring_buffer, buffer ) + end,
      sizeof( char ), &val );

	if( end >= sz ) {
		/* Circle around. */
      end = 0;
      meditprop(
         pid, mid,
         offsetof( struct ring_buffer, end ), sizeof( MEMLEN_T ), &end );
	} else {
      end++;
      meditprop(
         pid, mid,
         offsetof( struct ring_buffer, end ), sizeof( MEMLEN_T ), &end );
   }

	if( end + 1 == start ) {
		/* Overwrite old data. */
      start++;
      meditprop(
         pid, mid,
         offsetof( struct ring_buffer, start ), sizeof( MEMLEN_T ), &start );
	}
}

char ring_buffer_pop( uint8_t pid, MEM_ID mid ) {
   MEMLEN_T start = 0, end = 0, sz = 0;
   char out;

   mgetprop( pid, mid, offsetof( struct ring_buffer, start ),
      sizeof( MEMLEN_T ), &start );
   mgetprop( pid, mid, offsetof( struct ring_buffer, end ),
      sizeof( MEMLEN_T ), &end );
   assert( start != end );
   mgetprop( pid, mid, offsetof( struct ring_buffer, sz ),
      sizeof( MEMLEN_T ), &sz );
   mgetprop( pid, mid, offsetof( struct ring_buffer, buffer ) + start,
      sizeof( char ), &out );
   start++;
   if( start >= sz ) {
		/* Wrap around. */
      start = 0;
   }
   meditprop( pid, mid, offsetof( struct ring_buffer, start ),
      sizeof( MEMLEN_T ), &start );

   return out;
}

void ring_buffer_wait( struct ring_buffer* buffer ) {
   while( buffer->start == buffer->end ) {
      /* TODO: Handle wait. */
      io_call_handlers();
      /*irqal_call_handlers();
      mispos_suspend(); */
   }
}


