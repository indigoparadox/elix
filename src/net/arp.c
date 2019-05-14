
#include "arp.h"

#include "../debug.h"
#include "../mem.h"

#ifdef NET_CON_ECHO
void arp_print_packet( struct arp_packet* packet, int packet_len ) {
   int i = 0;
   uint8_t hwtype[2] = { 0 };
   uint8_t prototype[2] = { 0 };
   uint8_t* arp_packet_data = (uint8_t*)packet;
   char* buffer_p = (char*)packet;
   struct arp_header* header = (struct arp_header*)packet;

   *(uint16_t*)hwtype = ether_ntohs( header->hwtype );
   *(uint16_t*)prototype = ether_ntohs( header->prototype );

   printf( "   " );
   for( i = 0 ; packet_len > i ; i++ ) {
      printf( "%02X ", buffer_p[i] );
      if( 0 == i % 8 && 0 != i ) {
         printf( "\n   " );
      }
   }
   printf( "\n" );

   printf( "   *Packet Information:\n   Hardware: %02X %02X\n",
      hwtype[0], hwtype[1] );

   arp_packet_data += sizeof( struct arp_header );
   printf( "   Source MAC: " );
   for( i = 0 ; header->hwsize > i ; i++ ) {
      printf( "%02X ", *(arp_packet_data++) );
   }
#ifndef _WIN32
   printf( "\n   Source IP: " );
   for( i = 0 ; header->protosize > i ; i++ ) {
      printf( "%hhu ", *(arp_packet_data++) );
   }
#endif /* _WIN32 */
   printf( "\n   Dest MAC: " );
   for( i = 0 ; header->hwsize > i ; i++ ) {
      printf( "%02X ", *(arp_packet_data++) );
   }
#ifndef _WIN32
   printf( "\n   Dest IP: " );
   for( i = 0 ; header->protosize > i ; i++ ) {
      printf( "%hhu ", *(arp_packet_data++) );
   }
#endif /* _WIN32 */
   printf( "\n   Protocol: %02X %02X\n", prototype[0], prototype[1]  );
}
#endif /* NET_CON_ECHO */

int arp_get_dest_mac( uint8_t* mac, int mac_sz, struct arp_packet* arp ) {
   uint8_t* arp_packet_data = (uint8_t*)arp;
   int retval = 0;

   if( mac_sz < arp->header.hwsize ) {
      derror( "Provided buffer too small for hardware address" );
      goto cleanup;
   }

   arp_packet_data += sizeof( struct arp_header );
   arp_packet_data += arp->header.hwsize;
   arp_packet_data += arp->header.protosize;
   mcopy( mac, arp_packet_data, arp->header.hwsize );
   retval = arp->header.hwsize;

cleanup:
   return retval;
}

/* Accept the header because it could be any kind of ARP packet. */
int arp_respond( 
   struct arp_packet* call_packet, int call_packet_sz,
   struct arp_packet* resp_packet, int resp_packet_sz,
   uint8_t* my_mac, int my_mac_len, uint8_t* my_ip, int my_ip_len
) {
   uint8_t* arp_packet_data = (uint8_t*)call_packet;
   uint8_t* incoming_mac = NULL;
   uint8_t* incoming_ip = NULL;
   struct arp_header* header = &(call_packet->header);
   int packet_claimed_size = 0;
   int response_len = 0;

   if( my_ip_len != header->protosize || my_mac_len < header->hwsize ) {
      /* Weird address size. Nothing to do with us! */
      goto cleanup;
   }

#ifdef NET_CON_ECHO
   arp_print_packet( call_packet, call_packet_sz );
#endif /* NET_CON_ECHO */

   packet_claimed_size = 
      sizeof( struct arp_header ) + (2 * header->hwsize) +
      (2 * header->protosize);
   if( packet_claimed_size > call_packet_sz ) {
      deprintf(
         "Inconsistent packet size: %d claimed, %d actual.",
         packet_claimed_size, call_packet_sz );
      goto cleanup;
   }

   /* Bump the pointer out to the "target" layer-3 address and compare. */
   arp_packet_data += sizeof( struct arp_header );
   incoming_mac = arp_packet_data; /* Use for response below. */
   arp_packet_data += header->hwsize;
   incoming_ip = arp_packet_data; /* Use for response below. */
   arp_packet_data += header->protosize;
   arp_packet_data += header->hwsize;
   if( 0 != mcompare( arp_packet_data, my_ip, my_ip_len ) ) {
      goto cleanup;
   }

   if(
      NULL != resp_packet &&
      call_packet_sz > resp_packet_sz
   ) {
      derror( "Response buffer too small to respond" );
      goto cleanup;
   }

   response_len = packet_claimed_size;
   if( NULL != resp_packet ) {
      /* Create a response packet and fill it out. */
      mcopy( resp_packet, call_packet, response_len );
   } else {
      /* Reuse the call packet. */
      resp_packet = call_packet;
      resp_packet_sz = call_packet_sz;
   }
   resp_packet->header.opcode = ether_htons( ARP_REPLY );

   /* Move to the packet body and fill it out using info from above. Packets
    * should be near-identical in this case, due to testing above.
    */
   arp_packet_data = (uint8_t*)resp_packet;
   arp_packet_data += sizeof( struct arp_header );
   mcopy( arp_packet_data, my_mac, my_mac_len );
   arp_packet_data += header->hwsize;
   mcopy( arp_packet_data, my_ip, my_ip_len );
   arp_packet_data += header->protosize;
   mcopy( arp_packet_data, incoming_mac, header->hwsize );
   arp_packet_data += header->hwsize;
   mcopy( arp_packet_data, incoming_ip, header->protosize );

#ifdef NET_CON_ECHO
   printf( "It's me!\n" );
#endif /* NET_CON_ECHO */

cleanup:
   return response_len;
}

#if 0
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

   /* Fill out whatever addresses we were given. */

   if( NULL != src_mac ) {
      mcopy( arp_packet_out->src_mac, src_mac, ETHER_ADDRLEN );
   }

   if( NULL != dest_mac ) {
      mcopy( arp_packet_out->dest_mac, dest_mac, ETHER_ADDRLEN );
   }

   if( NULL != src_ip ) {
      mcopy( arp_packet_out->src_ip, src_ip, ETHER_ADDRLEN_IPV4 );
   }

   if( NULL != dest_ip ) {
      mcopy( arp_packet_out->dest_ip, dest_ip, ETHER_ADDRLEN_IPV4 );
   }

cleanup:
   return arp_packet_out;
}
#endif

