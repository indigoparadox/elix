
#include "ether.h"

#include "mem.h"

struct ether_frame* ether_new_frame(
   uint8_t src_mac[6], uint8_t dest_mac[6], enum ether_type type,
   void* data, size_t data_len
) {
   struct ether_frame* frame_out = NULL;
   struct ether_header* header_out = NULL;
   size_t frame_len = 0;
   
   /* Allocate the frame buffer and set the header to its start. */
   frame_len = sizeof( struct ether_header ) + data_len;
   frame_out = mem_alloc( 1, frame_len );
   header_out = (struct ether_header*)frame_out;
   if( NULL == frame_out ) {
      do_error( "Unable to allocate frame" );
      goto cleanup;
   }

   memcpy( header_out->src_mac, src_mac, 6 );
   memcpy( header_out->dest_mac, dest_mac, 6 );
   header_out->type = ether_htons( (uint16_t)type );
   memcpy( frame_out->data, data, data_len );

cleanup:
   return frame_out;
}

uint16_t ether_ntohs( const uint16_t input ) {
   uint8_t output[2] = { 0 };

   memcpy( &output, &input, sizeof( output ) );

   return
      ((uint16_t) output[1] << 0) |
      ((uint16_t) output[0] << 8);
}

uint32_t ether_ntohl( const uint32_t input ) {
   uint8_t output[4] = {};
   memcpy( &output, &input, sizeof( output ) );

   return
      ((uint32_t) output[3] << 0) |
      ((uint32_t) output[2] << 8) |
      ((uint32_t) output[1] << 16) |
      ((uint32_t) output[0] << 24);
}

