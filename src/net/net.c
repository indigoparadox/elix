
#include "net.h"
#include "arp.h"
#include "ether.h"
#include "../debug.h"
#include "../adhd.h"
#include "../mem.h"

#include <stddef.h>
#include <stdint.h>

/* Memory IDs for network tasks. */
#define NET_MID_SOCKET 1
#define NET_MID_RECEIVED 2
#define NET_MID_RESPONDED 3

uint8_t g_bcast_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
uint8_t g_src_ip[4] = { 10, 137, 2, 88 };
uint8_t g_search_ip[4] = { 10, 137, 2, 11 };
uint8_t g_src_mac[6] = { 0xab, 0xcd, 0xef, 0xde, 0xad, 0xbf };
char* g_ifname = "eth0";

int net_respond_arp_request(
   int pid, NET_SOCK socket, struct ether_frame* frame, int frame_len
) {
   struct arp_packet* arp = (struct arp_packet*)&(frame->data);
   int arp_len = 0;
   int arp_sz = frame_len - ether_get_header_len( frame, frame_len );
   int retval = 0;
   uint8_t dest_mac[6];
   int responded = 0;

   arp_len = arp_respond( arp, arp_sz, NULL, 0,
      g_src_mac, ETHER_ADDRLEN, g_src_ip, ETHER_ADDRLEN_IPV4 );
   if( 0 == arp_len ) {
      goto cleanup;
   }

   responded = mget_int( pid, NET_MID_RESPONDED );
   responded++;
   mset( pid, NET_MID_RESPONDED, &responded, sizeof( int ) );

#ifdef NET_CON_ECHO
   tputs( "Responding:\n" );
   arp_print_packet( arp, arp_len );
#endif /* NET_CON_ECHO */

   /* Send ARP response inside of a new ethernet frame. */
   arp_get_dest_mac( dest_mac, 6, arp );
   frame_len = ether_new_frame( frame, sizeof( struct ether_frame ),
      g_src_mac, dest_mac, ETHER_TYPE_ARP, arp, arp_len );
   if( 0 == frame_len ) {
      derror( "Unable to create frame" );
      retval = 1;
      goto cleanup;
   }

   net_send_frame( socket, frame, frame_len );

cleanup:
   return retval;
}

int net_respond_arp(
   int pid, NET_SOCK socket, struct ether_frame* frame, int frame_len
) {
   struct arp_header* arp_header = (struct arp_header*)&(frame->data);
   switch( ether_ntohs( arp_header->opcode ) ) {
      case ARP_REQUEST:
         return net_respond_arp_request( pid, socket, frame, frame_len );
   }
   return ARP_INVALID_PACKET;
}

int net_respond_task( int pid ) {
   struct ether_frame frame;
   int frame_len = 0;
   NET_SOCK* socket_ptr = NULL;
   NET_SOCK socket = NULL;
   int received = 0;

   socket = mget( pid, NET_MID_SOCKET, NULL );
   if( NULL == socket_ptr || NULL == *socket_ptr ) {
      socket = net_open_socket( g_ifname );
      mset( pid, NET_MID_SOCKET, &socket, sizeof( NET_SOCK ) );
   }

   frame_len = 
      net_poll_frame( socket, &frame, sizeof( struct ether_frame ) );
   if( 0 >= frame_len ) {
      goto cleanup;
   }

   received = mget_int( pid, NET_MID_RECEIVED );
   received++;
   mset( pid, NET_MID_RECEIVED, &received, sizeof( int ) );

#ifdef NET_CON_ECHO
   net_print_frame( &frame, frame_len );
#endif /* NET_CON_ECHO */
   switch( ether_ntohs( frame.header.type ) ) {
      case ETHER_TYPE_ARP:
         /* Shuck the Ethernet frame and handle the packet. */
         return net_respond_arp( pid, &socket, &frame, frame_len );
   }

cleanup:
   return 0;
}

