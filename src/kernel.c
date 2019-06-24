
#include <stdint.h>
#include <stddef.h>
#include "console.h"
#include "mem.h"
#include "net/net.h"
#include "adhd.h"
#define KERNEL_C
#include "kernel.h"
#include "alpha.h"

#ifndef CONSOLE_SERIAL
#include "display.h"
#include "keyboard.h"
#endif /* CONSOLE_SERIAL */

#define TASKS_MAX 5

const struct astring g_str_hello = astring_l( "hello\n" );
const struct astring g_str_stopping = astring_l( "stopping...\n" );

#include <stdio.h>
void kmain() {
   //uint8_t i = 0;
#ifndef SCHEDULE_COOP
   TASK_PID active = 0;
   TASK_RETVAL retval = 0;
#endif /* !SCHEDULE_COOP */

   minit();
   keyboard_init();
   display_init();
   /*
   for( i = 0 ; 4 > i ; i++ ) {
      uart_init( i );
   }
   */
   uart_init( 1 );
   net_init();

   adhd_start();
   adhd_launch_task( trepl_task );

   /* TODO: Kill task on request in COOP mode. */

#ifndef SCHEDULE_COOP
   while( SYSTEM_SHUTDOWN != g_system_state ) {
      for( active = 0 ; ADHD_TASKS_MAX > active ; active++ ) {
         retval = adhd_call_task( active );
         if( RETVAL_KILL == retval ) {
            /* Task returned -1; kill it. */
            adhd_kill_task( active );
         }
      }
   }
#endif /* !SCHEDULE_COOP */

   tputs( &g_str_stopping );

#ifndef CONSOLE_SERIAL
   keyboard_shutdown();
   display_shutdown();
#endif /* CONSOLE_SERIAL */

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

