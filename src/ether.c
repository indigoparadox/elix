
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

struct arp_packet_ipv4* ether_new_arp_packet_ipv4(
   enum arp_opcode op, const uint8_t* src_mac, const uint8_t* dest_mac,
   const uint8_t* src_ip, const uint8_t* dest_ip
) {
   struct arp_packet_ipv4* arp_packet_out = NULL;
   struct arp_header* arp_hdr_out = NULL;

   arp_packet_out = mem_alloc( 1, sizeof( struct arp_packet_ipv4 ) );
   if( NULL == arp_packet_out ) {
      perror( "Unable to allocate ARP packet" );
      goto cleanup;
   }
   arp_hdr_out = (struct arp_header*)arp_packet_out;

   arp_hdr_out->hwtype = ether_htons( (uint16_t)ETHER_TYPE_ETHER );
   arp_hdr_out->prototype = ether_htons( (uint16_t)ETHER_TYPE_IPV4 );
   arp_hdr_out->hwsize = 6;
   arp_hdr_out->protosize = 4;
   arp_hdr_out->opcode = ether_ntohs( (uint16_t)op );

   if( NULL != src_mac ) {
      memcpy( arp_packet_out->src_mac, src_mac, ETHER_ADDRLEN );
   }

   if( NULL != dest_mac ) {
      memcpy( arp_packet_out->dest_mac, dest_mac, ETHER_ADDRLEN );
   }

   if( NULL != src_ip ) {
      memcpy( arp_packet_out->src_ip, src_ip, ETHER_ADDRLEN_IPV4 );
   }

   if( NULL != dest_ip ) {
      memcpy( arp_packet_out->dest_ip, dest_ip, ETHER_ADDRLEN_IPV4 );
   }

cleanup:
   return arp_packet_out;
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

