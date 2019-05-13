
#include "arp.h"

#include <string.h>

#include "mem.h"

#ifdef NET_CON_ECHO
void arp_print_packet( struct arp_header* header, int packet_len ) {
   int i = 0;
   uint8_t hwtype[2] = { 0 };
   uint8_t prototype[2] = { 0 };
   bstring buffer = NULL;
   uint8_t* arp_packet_data = (uint8_t*)header;

   *(uint16_t*)hwtype = ether_ntohs( header->hwtype );
   *(uint16_t*)prototype = ether_ntohs( header->prototype );

   buffer = blk2bstr( header, packet_len );
   printf( "   " );
   for( i = 0 ; blength( buffer ) > i ; i++ ) {
      printf( "%02X ", (unsigned int)(unsigned char)bchar( buffer, i ) );
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
   printf( "\n   Source IP: " );
   for( i = 0 ; header->protosize > i ; i++ ) {
      printf( "%hhu ", *(arp_packet_data++) );
   }
   printf( "\n   Dest MAC: " );
   for( i = 0 ; header->hwsize > i ; i++ ) {
      printf( "%02X ", *(arp_packet_data++) );
   }
   printf( "\n   Dest IP: " );
   for( i = 0 ; header->protosize > i ; i++ ) {
      printf( "%hhu ", *(arp_packet_data++) );
   }
   printf( "\n   Protocol: %02X %02X\n", prototype[0], prototype[1]  );

/* cleanup: */
   bdestroy( buffer );
}
#endif /* NET_CON_ECHO */

uint8_t* arp_get_dest_mac( int* mac_len, struct arp_header* arp ) {
   uint8_t* arp_packet_data = (uint8_t*)arp;
   uint8_t* mac_out = NULL;

   *mac_len = arp->hwsize;
   mac_out = calloc( *mac_len, 1 );
   if( NULL == mac_out ) {
      perror( "Unable to allocate MAC buffer" );
      goto cleanup;
   }

   arp_packet_data += sizeof( struct arp_header );
   arp_packet_data += arp->hwsize;
   arp_packet_data += arp->protosize;
   memcpy( mac_out, arp_packet_data, arp->hwsize );

cleanup:
   return mac_out;
}

/* Accept the header because it could be any kind of ARP packet. */
struct arp_header* arp_respond( 
   struct arp_header* header, int packet_len,
   uint8_t* my_mac, int my_mac_len, uint8_t* my_ip, int my_ip_len,
   int* response_len
) {
   struct arp_header* response = NULL;
   uint8_t* arp_packet_data = (uint8_t*)header;
   uint8_t* incoming_mac = NULL;
   uint8_t* incoming_ip = NULL;

   *response_len = 0;

   if( my_ip_len != header->protosize || my_mac_len < header->hwsize ) {
      /* Weird address size. Nothing to do with us! */
      goto cleanup;
   }

#ifdef NET_CON_ECHO
   arp_print_packet( header, packet_len );
#endif /* NET_CON_ECHO */

   /* Bump the pointer out to the "target" layer-3 address and compare. */
   arp_packet_data += sizeof( struct arp_header );
   incoming_mac = arp_packet_data; /* Use for response below. */
   arp_packet_data += header->hwsize;
   incoming_ip = arp_packet_data; /* Use for response below. */
   arp_packet_data += header->protosize;
   arp_packet_data += header->hwsize;
   if( 0 != memcmp( arp_packet_data, my_ip, my_ip_len ) ) {
      goto cleanup;
   }

   /* Create a response packet and fill it out. */
   /* *response_len = (2 * header->hwsize) + (2 * header->protosize) +
      sizeof( struct arp_header ); */
   *response_len = packet_len;
   response = mem_alloc( 1, *response_len );
   memcpy( response, header, *response_len );
   response->opcode = ARP_REPLY;

   /* Move to the packet body and fill it out using info from above. Packets
    * should be near-identical in this case, due to testing above.
    */
   arp_packet_data = (uint8_t*)response;
   arp_packet_data += sizeof( struct arp_header );
   memcpy( arp_packet_data, my_mac, my_mac_len );
   arp_packet_data += header->hwsize;
   memcpy( arp_packet_data, my_ip, my_ip_len );
   arp_packet_data += header->protosize;
   memcpy( arp_packet_data, incoming_mac, header->hwsize );
   arp_packet_data += header->hwsize;
   memcpy( arp_packet_data, incoming_ip, header->protosize );

#ifdef NET_CON_ECHO
   printf( "It's me!\n" );
#endif /* NET_CON_ECHO */

cleanup:
   return response;
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

   /* Fill out whatever addresses we were given. */

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


