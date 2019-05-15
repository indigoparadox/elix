
#include <stdint.h>
#include <stddef.h>
#include "display.h"
#include "console.h"
#include "mem.h"
#include "debug.h"
#include "net/net.h"
#include "adhd.h"

#define TASKS_MAX 5

void kmain() {
   int retval = 0;
   int i = 0;

   minit();
   display_init();

   tputs( "hello\n" );

   /* Create network task. */
   adhd_add_task( net_respond_task );

   while( 1 ) {
      for( i = 0 ; adhd_get_tasks_len() > i ; i++ ) {
         retval = adhd_call_task( i );
      }
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

//cleanup:
   // TODO: net_close_socket( socket );
}

