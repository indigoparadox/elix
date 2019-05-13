
#include <stdint.h>
#include <stddef.h>
#include "bstrlib.h"
#include "net.h"
#include "ether.h"
#include "mem.h"

uint8_t g_bcast_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
uint8_t g_src_ip[4] = { 192, 168, 1, 22 };

int main( int argc, char** argv ) {
   int sockfd = 0;
   bstring if_name = NULL;
   //bstring send_buffer = NULL;
   uint8_t src_mac[6] = { 0 };
   struct ether_packet* packet = NULL;
   enum ether_type type = ETHER_TYPE_ARP;
   int retval = 0;
   int if_idx = 0;
   struct arp_packet_ipv4* arp = NULL;
   size_t packet_len = 0;

   if_name = bfromcstr( "eth0" );

   /* Open a socket. */
   sockfd = net_open_socket( if_name, &if_idx, src_mac );
   if( 0 > sockfd ) {
      do_error( "Unable to open socket" );
      retval = 1;
      goto cleanup;
   }

   /* Create an ARP request. */
   arp = ether_new_arp_packet_ipv4(
      ARP_REQUEST, src_mac, g_bcast_mac, g_src_ip, NULL );

   /* Create a packet using the MAC from the socket above. */
   packet = ether_new_packet(
      src_mac, g_bcast_mac, type, arp, sizeof( struct arp_packet_ipv4 ) );
   if( NULL == packet ) {
      do_error( "Unable to create packet" );
      retval = 1;
      goto cleanup;
   }

   /* Flatten the packet into a buffer and send it. */
   packet_len =
      sizeof( struct ether_header ) + sizeof( struct arp_packet_ipv4 );
   /*send_buffer = blk2bstr( packet, packet_len );
   if( NULL == send_buffer ) {
      perror( "Unable to allocate packet data buffer" );
      goto cleanup;
   }*/
   net_send_packet( sockfd, if_idx, packet, packet_len );

cleanup:
   mem_free( packet );
   return retval;
}

