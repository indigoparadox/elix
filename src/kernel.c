
#include <stdint.h>
#include <stddef.h>
#include "display.h"
#include "console.h"
#include "net/net.h"
#include "net/ether.h"
#include "mem.h"
#include "net/arp.h"
#include "debug.h"

uint8_t g_bcast_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
//uint8_t g_bcast_mac[6] = { 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff };
uint8_t g_src_ip[4] = { 10, 137, 2, 88 };
uint8_t g_search_ip[4] = { 10, 137, 2, 11 };
uint8_t g_src_mac[6] = { 0xab, 0xcd, 0xef, 0xde, 0xad, 0xbf };
//uint8_t g_src_mac[6] = { 0x00, 0x16, 0x3E, 0x5E, 0x6C, 0x09 };
char* g_ifname = "eth0";


void kmain() {
   struct ether_frame frame;
   int frame_len = 0;
   enum ether_type type = ETHER_TYPE_ARP;
   int retval = 0;
   struct arp_packet arp;
   int arp_len = 0;
   int response_len = 0;
   int mac_len = 0;
   NET_SOCK socket = NULL;
   uint8_t dest_mac[6];
   struct arp_header* arp_header = NULL;

   display_init();

   tputs( "hello\n" );

   /* Open a socket. */
   socket = net_open_socket( g_ifname );
   if( NULL == socket ) {
      derror( "Unable to open socket" );
      retval = 1;
      goto cleanup;
   }

#if 0
   /* Create an ARP request. */
   arp_len = arp_new_packet_ipv4(
      &arp, ARP_PACKET_SIZE_MAX,
      ARP_REQUEST, g_src_mac, g_bcast_mac, g_src_ip, g_search_ip );

   /* Create a frame using the MAC from the socket above. */
   frame_len = ether_new_frame(
      &frame, sizeof( struct ether_frame ),
      g_src_mac, g_bcast_mac, type,
      &arp, sizeof( struct arp_packet ) );
   if( 0 == frame_len ) {
      derror( "Unable to create frame" );
      retval = 1;
      goto cleanup;
   }

   /* Flatten the frame into a buffer and send it. */
   net_print_frame( &frame, frame_len );
   net_send_frame( socket, &frame, frame_len );
#endif
   /* Listen for and handle incoming packets. */
   while( 1 ) {
      frame_len = 
         net_poll_frame( socket, &frame, sizeof( struct ether_frame ) );
      if( 0 < frame_len ) {
#ifdef NET_CON_ECHO
         net_print_frame( &frame, frame_len );
#endif /* NET_CON_ECHO */
         switch( ether_ntohs( frame.header.type ) ) {
            case ETHER_TYPE_ARP:
               /* Shuck the Ethernet frame and handle the packet. */
               arp_header = (struct arp_header*)&(frame.data);
               if( ARP_REQUEST == ether_ntohs( arp_header->opcode ) ) {
                  arp_len = arp_respond( 
                     (struct arp_packet*)&(frame.data),
                     frame_len - ether_get_header_len( &frame, frame_len ),
                     NULL, 0,
                     g_src_mac, ETHER_ADDRLEN, g_src_ip, ETHER_ADDRLEN_IPV4 );
                  if( 0 == arp_len ) {
                     continue;
                  }

#ifdef NET_CON_ECHO
                  printf( "Responding:\n" );
                  arp_print_packet( &arp, arp_len );
#endif /* NET_CON_ECHO */

                  /* Send ARP response inside of a new ethernet frame. */
                  arp_get_dest_mac( dest_mac, 6, &arp );
                  frame_len = ether_new_frame(
                     &frame, sizeof( struct ether_frame ),
                     g_src_mac, dest_mac, type,
                     &arp, arp_len );
                  if( 0 == frame_len ) {
                     derror( "Unable to create frame" );
                     retval = 1;
                     goto cleanup;
                  }
                  net_send_frame( socket, &frame, frame_len );

               }
               continue;
         }
      }
   }

cleanup:
   net_close_socket( socket );
}

