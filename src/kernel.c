
#include "code16.h"

#include "etypes.h"

#if defined( USE_EXT_CLI ) && !defined( USE_REPL )
#error "repl app is required to enable external CLI!"
#endif /* USE_EXT_CLI && !USE_REPL */

#define KERNEL_C
#include "kernel.h"
#include "console.h"
#include "mem.h"
#include "net/net.h"
#include "adhd.h"
#include "alpha.h"

TASK_RETVAL repl_command( const struct astring* cli );
TASK_RETVAL trepl_task();

#define TASKS_MAX 5

#ifdef CONSOLE_UART_WO
#include "uart.h"
#endif /* CONSOLE_UART_WO */

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
   struct astring* cli = NULL;
   char c = 0;
   int i = 0, j = 0;
   bool switch_found = false;
   bool do_init = true;
   bool cmd_found = false;
#ifdef CONSOLE_UART_WO
   uint8_t i = 0;
#endif /* CONSOLE_UART_WO */

   if( 1 < argc ) {
      cli = alpha_astring(
         PID_MAIN, KERNEL_MID_CLI, 30, NULL );

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
               alpha_astring_append( cli, c );
            }
         }
         if( i + 1 < argc ) {
            if( switch_found ) {
               switch_found = false;
            } else {
               alpha_astring_append( cli, ' ' );
            }
         }
      }
   }

   if( do_init ) {
#endif /* USE_EXT_CLI */

#ifdef CRASH
   int k = 1, l;
   do {
      l = 1 / k;
      k--;
   } while( k > 0 || l > 0 );
#endif /* CRASH */

#ifdef CONSOLE_UART_WO
   uart_init_all();

   for( i = 0 ; 30 > i ; i++ ) {
      uart_putc( 1, '.' );
   }
   uart_putc( 1, '\n' );
   
   /* MSP430 crashes somewhere around here. */

#endif /* CONSOLE_UART_WO */

#ifdef USE_NET
   net_init();
#endif /* USE_NET */

   adhd_start();

#ifdef USE_REPL
   adhd_launch_task( trepl_task );
#endif /* USE_REPL */

#ifdef USE_EXT_CLI
   }

   if( cmd_found ) {
      retval = repl_command( cli );
      goto cleanup;
   }
#endif /* USE_EXT_CLI */

   /* TODO: Kill task on request in COOP mode. */

/*
   P1DIR = BIT0;
   P1OUT = BIT0;

   uint8_t c = 'a';
      
   while( 1 ) {
      if( 'z' <= c ) {
         c = 'a';
      }
      uart_putc( 1, c++ );
   }
*/

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

