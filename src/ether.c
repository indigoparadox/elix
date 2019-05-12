
#include "ether.h"

#include "mem.h"

struct ether_packet* ether_new_packet(
   uint8_t src_mac[6], uint8_t dest_mac[6], uint16_t type, bstring data
) {
   struct ether_packet* packet_out = NULL;
   struct ether_header* header_out = NULL;
   char* data_c;
   size_t data_len;
   
   /* Make sure the data buffer will fit inside of a packet. */
   if( blength( data ) > ETHER_BUFFER_SIZE ) {
      do_error( "Data too large for packet" );
      goto cleanup;
   }

   /* Allocate the packet buffer and set the header to its start. */
   packet_out = mem_alloc( 1, sizeof( struct ether_packet ) );
   header_out = (struct ether_header*)packet_out;
   if( NULL == packet_out ) {
      do_error( "Unable to allocate packet" );
      goto cleanup;
   }

   memcpy( header_out->src_mac, src_mac, 6 );
   memcpy( header_out->dest_mac, dest_mac, 6 );
   header_out->type = type;
   data_c = bdata( data );
   data_len = blength( data );
   strncpy( packet_out->data, data_c, data_len );

cleanup:
   return packet_out;
}

