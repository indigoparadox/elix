
#include "arp.h"

#include <string.h>

#include "mem.h"

void ether_arp_respond( struct arp_packet_ipv4* packet ) {

}

struct arp_packet_ipv4* arp_new_packet_ipv4(
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


