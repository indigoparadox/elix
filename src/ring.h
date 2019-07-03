
#ifndef RING_H
#define RING_H

#include <stdint.h>

#include "mem.h"

struct ring_buffer {
   MEMLEN_T sz;
   MEMLEN_T start;
   MEMLEN_T end;
   char buffer[];
};

const struct ring_buffer* ring_buffer( uint8_t pid, MEM_ID mid, MEMLEN_T sz );
void ring_buffer_push( uint8_t pid, MEM_ID mid, char val );
char ring_buffer_pop( uint8_t pid, MEM_ID mid );
void ring_buffer_wait( struct ring_buffer* buffer );

#endif /* RING_H */

