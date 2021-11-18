
#include "code16.h"

#include "etypes.h"

#define KERNEL_C
#define IO_C
#include "kernel.h"
#include "io.h"
#include "console.h"
#include "mem.h"
#include "net/net.h"
#include "adhd.h"

TASK_RETVAL INIT_TASK();

#define TASKS_MAX 5

#ifdef USE_ERROR_CODES
static const char gc_invalid_init[] = "EM01\n";
#else
static const char gc_invalid_init[] = "invalid init\n";
#endif /* USE_ERROR_CODES */

#ifdef USE_EXT_CLI
int kmain( int argc, char** argv ) {
#else
int kmain() {
#endif /* USE_EXT_CLI */
   TASK_PID active = 0;
   int fg_pid = -1;
#ifdef USE_EXT_CLI
   struct astring* cli = NULL;
   char c = 0;
   int i = 0, j = 0;
   bool switch_found = false;
   bool do_init = true;
   bool cmd_found = false;
#endif /* USE_EXT_CLI */
   FILEPTR_T init_offset = 0;
   TASK_PID init_pid = 0;

   minit();

#ifdef USE_EXT_CLI
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

   tinput_init();
   toutput_init();

#ifdef USE_NET
   net_init();
#endif /* USE_NET */ 

   #define quote_me( str ) #str
   #define string_me( str ) quote_me( str )
   #define INIT_TASK_GID_STR string_me( INIT_TASK_GID )

   adhd_init();

   init_offset = mfat_get_root_dir_offset( 0, 0 );
   init_offset = mfat_get_dir_entry_offset(
      "INIT.ELX", MFAT_FILENAME_LEN, init_offset, 0, 0 );
   if( 0 == init_offset ) {
      tprintf( "%s\n", gc_invalid_init );
      goto cleanup;
   }
   init_pid = adhd_task_launch( 0, 0, init_offset );
   if( ADHD_ERROR_TASK_NOT_FOUND == init_pid ) {
      tprintf( "%s\n", gc_invalid_init );
      goto cleanup;
   }

#ifdef USE_EXT_CLI
   }

   if( cmd_found ) {
      // TODO
      //retval = repl_command( cli );
      goto cleanup;
   }
#endif /* USE_EXT_CLI */

   while( SYSTEM_SHUTDOWN != g_system_state ) {
      fg_pid = -1;
      for( active = 0 ; ADHD_TASKS_MAX > active ; active++ ) {
         if( 0 < g_tasks[active].proc.ipc ) {
            if( g_tasks[active].flags & ADHD_TASK_FLAG_FOREGROUND ) {
               fg_pid = active;
            }
            adhd_task_execute_next( active );
         }
      }
      if( 0 > fg_pid ) {
         /* We need at least one foreground process. */
         g_tasks[init_pid].flags |= ADHD_TASK_FLAG_FOREGROUND;
      } else if( init_pid != fg_pid ) {
         /* Shut off init I/O if something else asked for FG. */
         /* TODO: More nuanced I/O control. */
         g_tasks[init_pid].flags &= ~ADHD_TASK_FLAG_FOREGROUND;
      }
      if( 0 == g_tasks[init_pid].proc.ipc ) {
         tprintf( "init died; shutting down\n" );
         g_system_state = SYSTEM_SHUTDOWN;
      }
   }

   tprintf( "stopping...\n" );

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

cleanup:

   return 0;
}

