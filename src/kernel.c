
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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

TASK_RETVAL repl_command( const struct astring* cli );

#define TASKS_MAX 5

#ifdef USE_EXT_CLI
int kmain( int argc, char** argv ) {
#else
int kmain() {
#endif /* USE_EXT_CLI */
#ifndef SCHEDULE_COOP
   TASK_PID active = 0;
   TASK_RETVAL retval = 0;
#endif /* !SCHEDULE_COOP */
#ifdef USE_EXT_CLI
   const struct astring* cli = NULL;
   char c = 0;
   int i = 0, j = 0;
   bool switch_found = false;
   bool do_init = true;
   bool cmd_found = false;

   if( 1 < argc ) {
      cli = alpha_astring(
         ADHD_PID_MAIN, KERNEL_MID_CLI, 30, NULL );

      for( i = 1 ; argc > i ; i++ ) {
         j = 0;
         while( '\0' != (c = argv[i][j++]) ) {
            if( '-' == c ) {
               switch_found = true;
            } else if( switch_found ) {
               switch( c ) {
               case 'n':
                  do_init = false;
                  break;
               }
            } else {
               cmd_found = true;
               alpha_astring_append( ADHD_PID_MAIN, KERNEL_MID_CLI, c );
            }
         }
         if( i + 1 < argc ) {
            if( switch_found ) {
               switch_found = false;
            } else {
               alpha_astring_append( ADHD_PID_MAIN, KERNEL_MID_CLI, ' ' );
            }
         }
      }
   }

   if( do_init ) {
#endif /* USE_EXT_CLI */

#ifdef USE_NET
   net_init();
#endif /* USE_NET */

   adhd_start();

#ifdef USE_EXT_CLI
   }

   if( cmd_found ) {
      retval = repl_command( cli );
      goto cleanup;
   }
#endif /* USE_EXT_CLI */

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

   tprintf( "stopping..." CONSOLE_NEWLINE );

#ifdef USE_EXT_CLI
cleanup:
   if( do_init ) {
#endif /* USE_EXT_CLI */

#ifdef USE_EXT_CLI
   }
#endif /* USE_EXT_CLI */

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
   // TODO: net_close_socket( socket );

   return 0;
}

