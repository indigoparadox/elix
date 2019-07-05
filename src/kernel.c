
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

#ifdef USE_EXT_CLI
#include "commands.h"
#endif /* USE_EXT_CLI */

#define TASKS_MAX 5

const struct astring g_str_hello = astring_l( "hello\n" );
const struct astring g_str_stopping = astring_l( "stopping...\n" );

#ifdef USE_EXT_CLI
int kmain( int argc, char** argv ) {
#else
int kmain() {
#endif /* USE_EXT_CLI */
   uint8_t i = 0;
#ifndef SCHEDULE_COOP
   TASK_PID active = 0;
   TASK_RETVAL retval = 0;
#endif /* !SCHEDULE_COOP */
#ifdef USE_EXT_CLI
   const struct astring* cli;
   char c = 0;
   int j = 0;
   bool switch_found = false;
   bool do_init = true;
#endif /* USE_EXT_CLI */

   minit();

#ifdef USE_EXT_CLI
   if( 1 < argc ) {
      cli = alpha_astring(
         ADHD_PID_MAIN, KERNEL_MID_CLI, REPL_LINE_SIZE_MAX + 1, NULL );

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
      
      retval = do_command( cli );

      goto cleanup;
   }

   if( do_init ) {
#endif /* USE_EXT_CLI */

   keyboard_init();
   display_init();
   for( i = 0 ; 4 > i ; i++ ) {
      uart_init( i );
   }
   net_init();

   adhd_start();

#ifdef USE_EXT_CLI
   }
#endif /* USE_EXT_CLI */

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

#ifdef USE_EXT_CLI
cleanup:
   if( do_init ) {
#endif /* USE_EXT_CLI */

#ifndef CONSOLE_SERIAL
   keyboard_shutdown();
   display_shutdown();
#endif /* CONSOLE_SERIAL */

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

