
#ifndef RING_H
#define RING_H

#include <stdint.h>

struct ring_buffer {
   uint8_t* buffer;
   uint8_t start;
   uint8_t end;
   uint8_t len;
};

void ring_buffer_init(
   struct ring_buffer* info, uint8_t* buffer, uint8_t len
);
void ring_buffer_push( uint8_t val, struct ring_buffer* buffer );
uint8_t ring_buffer_pop( struct ring_buffer* buffer );
void ring_buffer_wait( struct ring_buffer* buffer );

#endif /* RING_H */

