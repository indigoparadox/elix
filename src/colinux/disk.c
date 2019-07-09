
#include "../disk.h"

#ifndef CHECK

#include <sys/mman.h>

extern uint8_t* g_img_map;
extern size_t g_img_sz;

uint8_t disk_get_byte( uint8_t dev_idx, uint8_t part_idx, size_t offset ) {
   return g_img_map[offset];
}

void disk_set_byte(
   uint8_t byte, uint8_t dev_idx, uint8_t part_idx, size_t offset
) {
   g_img_map[offset] = byte;
   msync( g_img_map, g_img_sz, MS_SYNC );
}

uint16_t read_int16_msb_first( uint16_t* num ) {
   uint16_t out = 0;
   uint8_t* bytes_ptr = (uint8_t*)num;
   int i = 0;
   
   for( i = 1 ; 0 <= i ; i-- ) {
      out |= bytes_ptr[i];
      out <<= 8;
   }

   return out;
}

#endif /* CHECK */

