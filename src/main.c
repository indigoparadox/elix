
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
   struct ether_frame* frame = NULL;
   enum ether_type type = ETHER_TYPE_ARP;
   int retval = 0;
   int if_idx = 0;
   struct arp_packet_ipv4* arp = NULL;
   size_t frame_len = 0;

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

   /* Create a frame using the MAC from the socket above. */
   frame = ether_new_frame(
      src_mac, g_bcast_mac, type, arp, sizeof( struct arp_packet_ipv4 ) );
   if( NULL == frame ) {
      do_error( "Unable to create frame" );
      retval = 1;
      goto cleanup;
   }

   /* Flatten the frame into a buffer and send it. */
   frame_len =
      sizeof( struct ether_header ) + sizeof( struct arp_packet_ipv4 );
   /*send_buffer = blk2bstr( packet, packet_len );
   if( NULL == send_buffer ) {
      perror( "Unable to allocate packet data buffer" );
      goto cleanup;
   }*/
   net_send_frame( sockfd, if_idx, frame, frame_len );
   mem_free( frame );

   while( 1 ) {
      frame = net_poll_frame( sockfd );
      if( NULL != frame ) {
         net_print_frame( frame, sizeof( struct ether_header ) );
      }
   }

cleanup:
   mem_free( frame );
   net_close_socket( sockfd );
   return retval;
}

