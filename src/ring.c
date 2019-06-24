
#include "ring.h"

#include "io.h"

void ring_buffer_init(
   struct ring_buffer* info, uint8_t* buffer, uint8_t len
) {
   /* XXX: FIXME */
   /*info->buffer = buffer;
   info->start = 0;
   info->end = 0;
   info->len = len;*/
}

void ring_buffer_push( uint8_t val, struct ring_buffer* buffer ) {
   buffer->buffer[buffer->end++] = val;
	if( buffer->len <= buffer->end ) {
		/* Circle around. */
		buffer->end = 0;
	}

	if( buffer->end + 1 == buffer->start ) {
		/* Overwrite old data. */
		buffer->start++;
	}
}

uint8_t ring_buffer_pop( struct ring_buffer* buffer ) {
   uint8_t out;
	out = buffer->buffer[buffer->start++];
	if( buffer->len <= buffer->start ) {
		/* Wrap around. */
		buffer->start = 0;
	}
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


