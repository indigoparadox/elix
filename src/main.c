
#include <stdint.h>
#include <stddef.h>
#include "bstrlib.h"
#include "net.h"
#include "ether.h"
#include "mem.h"
#include "arp.h"

uint8_t g_bcast_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
uint8_t g_src_ip[4] = { 10, 137, 2, 88 };
uint8_t g_search_ip[4] = { 10, 137, 2, 11 };

int main( int argc, char** argv ) {
   int sockfd = 0;
   bstring if_name = NULL;
   uint8_t src_mac[6] = { 0 };
   struct ether_frame* frame = NULL;
   enum ether_type type = ETHER_TYPE_ARP;
   int retval = 0;
   int if_idx = 0;
   struct arp_packet_ipv4* arp = NULL;
   int frame_len = 0;
   int response_len = 0;
   int mac_len = 0;

   if_name = bfromcstr( "eth0" );

   /* Open a socket. */
   sockfd = net_open_socket( if_name, &if_idx, src_mac );
   if( 0 > sockfd ) {
      do_error( "Unable to open socket" );
      retval = 1;
      goto cleanup;
   }

   /* Create an ARP request. */
   arp = arp_new_packet_ipv4(
      ARP_REQUEST, src_mac, g_bcast_mac, g_src_ip, g_search_ip );

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
   net_send_frame( sockfd, if_idx, frame, frame_len );
   mem_free( frame );

   /* Listen for and handle incoming packets. */
   while( 1 ) {
      frame = net_poll_frame( sockfd, &frame_len );
      if( NULL != frame ) {
         net_print_frame( frame, frame_len );
         switch( ether_ntohs( frame->header.type ) ) {
            case ETHER_TYPE_ARP:
               /* Shuck the Ethernet frame and handle the packet. */
               if(
                  ARP_REQUEST == ether_ntohs(
                     ((struct arp_header*)(frame->data))->opcode )
               ) {
                  arp = arp_respond( 
                     (struct arp_header*)(frame->data),
                     frame_len - sizeof( struct ether_header ),
                     src_mac, ETHER_ADDRLEN, g_src_ip, ETHER_ADDRLEN_IPV4,
                     &response_len );

                  mem_free( frame );
                  if( NULL == arp ) {
                     continue;
                  }

                  /* Send ARP response inside of a new ethernet frame. */
                  frame = ether_new_frame(
                     src_mac, arp_get_dest_mac( &mac_len, arp ), type, arp,
                     sizeof( struct arp_packet_ipv4 ) );
                  if( NULL == frame ) {
                     do_error( "Unable to create frame" );
                     retval = 1;
                     goto cleanup;
                  }
                  net_send_frame( sockfd, if_idx, frame, frame_len );

               }
               continue;

            default:
               mem_free( frame );
               continue;
         }
      }
   }

cleanup:
   mem_free( frame );
   net_close_socket( sockfd );
   return retval;
}

