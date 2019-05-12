
#include "ether.h"

#include "mem.h"

struct ether_packet* ether_new_packet() {
   struct ether_packet* packet_out = NULL;
   struct ether_header* header_out = NULL;
   
   /* Allocate the packet buffer and set the header to its start. */
   packet_out = mem_alloc( 1, sizeof( struct ether_packet ) );
   header_out = (struct ether_header*)packet_out;
   if( NULL == packet_out ) {
      return NULL;
   }

   //header_out

   return packet_out;
}

