
#include <stdint.h>
#include <stddef.h>
#include "bstrlib.h"
#include "net.h"
#include "ether.h"
#include "mem.h"
#include "arp.h"

uint8_t g_bcast_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
//uint8_t g_bcast_mac[6] = { 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff };
uint8_t g_src_ip[4] = { 10, 137, 2, 88 };
uint8_t g_search_ip[4] = { 10, 137, 2, 11 };
uint8_t g_src_mac[6] = { 0xab, 0xcd, 0xef, 0xde, 0xad, 0xbf };
//uint8_t g_src_mac[6] = { 0x00, 0x16, 0x3E, 0x5E, 0x6C, 0x09 };

int main( int argc, char** argv ) {
   bstring if_name = NULL;
   struct ether_frame* frame = NULL;
   enum ether_type type = ETHER_TYPE_ARP;
   int retval = 0;
   struct arp_header* arp = NULL;
   int frame_len = 0;
   int response_len = 0;
   int mac_len = 0;
   NET_SOCK socket = NULL;

   if_name = bfromcstr( "eth0" );

   /* Open a socket. */
   socket = net_open_socket( if_name );
   if( NULL == socket ) {
      do_error( "Unable to open socket" );
      retval = 1;
      goto cleanup;
   }

   /* Create an ARP request. */
   arp = (struct arp_header*)arp_new_packet_ipv4(
      ARP_REQUEST, g_src_mac, g_bcast_mac, g_src_ip, g_search_ip );

   /* Create a frame using the MAC from the socket above. */
   frame = ether_new_frame(
      g_src_mac, g_bcast_mac, type, arp, sizeof( struct arp_packet_ipv4 ) );
   if( NULL == frame ) {
      do_error( "Unable to create frame" );
      retval = 1;
      goto cleanup;
   }

   /* Flatten the frame into a buffer and send it. */
   frame_len =
      sizeof( struct ether_header ) + sizeof( struct arp_packet_ipv4 );
   net_print_frame( frame, frame_len );
   net_send_frame( socket, frame, frame_len );
   mem_free( frame );

   /* Listen for and handle incoming packets. */
   while( 1 ) {
      frame = net_poll_frame( socket, &frame_len );
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
                     g_src_mac, ETHER_ADDRLEN, g_src_ip, ETHER_ADDRLEN_IPV4,
                     &response_len );

                  mem_free( frame );
                  if( NULL == arp ) {
                     continue;
                  }

                  printf( "Responding:\n" );
                  arp_print_packet( arp, response_len );

                  /* Send ARP response inside of a new ethernet frame. */
                  frame = ether_new_frame(
                     g_src_mac, arp_get_dest_mac(
                        &mac_len, (struct arp_header*)arp ), type, arp,
                     sizeof( struct arp_packet_ipv4 ) );
                  if( NULL == frame ) {
                     do_error( "Unable to create frame" );
                     retval = 1;
                     goto cleanup;
                  }
                  net_send_frame( socket, frame, frame_len );

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
   net_close_socket( socket );
   return retval;
}

