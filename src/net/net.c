
#define NET_C
#include "net.h"
#include "arp.h"
#include "ether.h"
#include "../adhd.h"
#include "../mem.h"
#include "../alpha.h"

/* TODO: Move interactive net stuff into its own module so it doesn't depend
 * on strings and console. */
#include "../console.h"
#include "../strings.h"

#include <stddef.h>
#include <stdint.h>

/* Memory IDs for network tasks. */
#define NET_MID_SOCKET 1

const uint8_t g_bcast_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
const uint8_t g_src_ip[4] = { 10, 137, 2, 88 };
const uint8_t g_search_ip[4] = { 10, 137, 2, 11 };
const uint8_t g_src_mac[6] = { 0xab, 0xcd, 0xef, 0xde, 0xad, 0xbf };
const char* g_ifname = "eth0";

const struct astring g_str_netp = astring_l( "netp" );

uint8_t net_respond_arp_request(
   TASK_PID pid, NET_SOCK socket, struct ether_frame* frame, int frame_len
) {
   struct arp_packet* arp = (struct arp_packet*)&(frame->data);
   int arp_len = 0;
   int arp_sz = frame_len - ether_get_header_len( frame, frame_len );
   uint8_t retval = 0;
   uint8_t dest_mac[6];

   arp_len = arp_respond( arp, arp_sz, NULL, 0,
      g_src_mac, ETHER_ADDRLEN, g_src_ip, ETHER_ADDRLEN_IPV4 );
   if( 0 == arp_len ) {
      goto cleanup;
   }

   mincr( pid, NET_MID_RESPONDED );

#ifdef NET_CON_ECHO
   tputs( g_str_responding );
   arp_print_packet( arp, arp_len );
#endif /* NET_CON_ECHO */

   /* Send ARP response inside of a new ethernet frame. */
   arp_get_dest_mac( dest_mac, 6, arp );
   frame_len = ether_new_frame( frame, sizeof( struct ether_frame ),
      g_src_mac, dest_mac, ETHER_TYPE_ARP, arp, arp_len );
   if( 0 == frame_len ) {
      retval = 1;
      goto cleanup;
   }

   net_send_frame( socket, frame, frame_len );

cleanup:
   return retval;
}

uint8_t net_respond_arp(
   TASK_PID pid, NET_SOCK socket, struct ether_frame* frame, int frame_len
) {
   struct arp_header* arp_header = (struct arp_header*)&(frame->data);
   switch( ether_ntohs( arp_header->opcode ) ) {
      case ARP_REQUEST:
         return net_respond_arp_request( pid, socket, frame, frame_len );
   }
   return ARP_INVALID_PACKET;
}

static bool net_create_socket() {
   NET_SOCK sock_tmp = NULL;

   /* TODO: Get ifname from command line. */
   sock_tmp = net_open_socket( g_ifname );
   if( NULL == sock_tmp ) {
      return true;
   }

   mset( adhd_get_pid(), NET_MID_SOCKET, sizeof( NET_SOCK ), &sock_tmp );

   return false;
}

TASK_RETVAL net_respond_task() {
   struct ether_frame frame;
   int frame_len = 0;
   const NET_SOCK* socket = NULL;

   adhd_task_setup();

   adhd_set_gid( &g_str_netp );

   /* Try to get the socket. If we fail, create it and continue. */
   socket = mget( adhd_get_pid(), NET_MID_SOCKET, sizeof( NET_SOCK ) );
   if( NULL == *socket && net_create_socket() ) {
      /* Couldn't create it. */
      tprintf( "no socket\n" );
      adhd_exit_task();
   }

   frame_len = 
      net_poll_frame( *socket, &frame, sizeof( struct ether_frame ) );
   if( 0 >= frame_len ) {
      adhd_yield();
      adhd_continue_loop();
   }

   mincr( adhd_get_pid(), NET_MID_RECEIVED );

#ifdef NET_CON_ECHO
   net_print_frame( &frame, frame_len );
#endif /* NET_CON_ECHO */
   switch( ether_ntohs( frame.header.type ) ) {
      case ETHER_TYPE_ARP:
         /* Shuck the Ethernet frame and handle the packet. */
         net_respond_arp( adhd_get_pid(), &socket, &frame, frame_len );
         adhd_yield();
         adhd_continue_loop();
   }

   adhd_yield();
   adhd_end_loop();
}

void net_init() {

}

