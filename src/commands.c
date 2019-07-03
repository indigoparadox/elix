
#ifdef USE_CONSOLE

#include "commands.h"

#include "net/net.h"
#include "console.h"
#include "kernel.h"
#include "mfat.h"

/* = Command Callbacks */

extern struct astring g_str_netp;
extern MEMLEN_T g_mheap_top;

static TASK_RETVAL tnet_start( const struct astring* cli ) {
   tprintf( "start?\n" );
   adhd_launch_task( net_respond_task );
   return RETVAL_OK;
}

static TASK_RETVAL tnet_rcvd( const struct astring* cli ) {
   const int* received = NULL;
   TASK_PID pid;

   pid = adhd_get_pid_by_gid( &g_str_netp );
   received = mget( pid, NET_MID_RECEIVED, sizeof( int ) );
   tprintf( "frames rcvd: %d\n", *received );

   return RETVAL_OK;
}

#define NET_COMMANDS_COUNT 2
const struct command g_net_commands[NET_COMMANDS_COUNT] = {
   { "rcvd", tnet_rcvd },
   { "start", tnet_start },
};

static TASK_RETVAL trepl_net( const struct astring* cli ) {
   const char* tok;
   uint8_t i = 0;

   tok = alpha_tok( cli, ' ', 1 );
   if( NULL == tok ) {
      return 1;
   }

   for( i = 0 ; NET_COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp_cc(
         tok, CMD_MAX_LEN, g_net_commands[i].command, CMD_MAX_LEN, ' '
      ) ) {
         return g_net_commands[i].callback( cli );
      }
   }

   return RETVAL_BAD_ARGS;
}

static TASK_RETVAL tsys_exit( const struct astring* cli ) {
   g_system_state = SYSTEM_SHUTDOWN;
   return RETVAL_OK;
}

static TASK_RETVAL tsys_mem( const struct astring* cli ) {
   tprintf(
      "mem:\nused: %d\ntotal: %d\nfree: %d\n",
      get_mem_used(), MEM_HEAP_SIZE, MEM_HEAP_SIZE - get_mem_used()
   );
   return RETVAL_OK;
}

#define SYS_COMMANDS_COUNT 2
const struct command g_sys_commands[SYS_COMMANDS_COUNT] = {
   { "exit", tsys_exit },
   { "mem", tsys_mem }
};

static TASK_RETVAL trepl_sys( const struct astring* cli ) {
   const char* tok;
   uint8_t i = 0;

   tok = alpha_tok( cli, ' ', 1 );
   if( NULL == tok ) {
      return 1;
   }

   for( i = 0 ; SYS_COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp_cc(
         tok, CMD_MAX_LEN, g_sys_commands[i].command, CMD_MAX_LEN, ' '
      ) ) {
         return g_sys_commands[i].callback( cli );
      }
   }

   return RETVAL_BAD_ARGS;
}

#ifdef USE_DISK

static TASK_RETVAL tdisk_dir( const struct astring* cli ) {
   uint16_t offset = 0;
   char filename[12] = { 0 };

   offset = mfat_get_root_dir_offset( 0, 0 );
   
   do {
      mfat_get_dir_entry_name( filename, offset, 0, 0 );
      tprintf( "- %s\n", filename );
      offset = mfat_get_dir_entry_next_offset( offset, 0, 0 );
   } while( 0 < offset );

   return RETVAL_OK;
}

#define DISK_COMMANDS_COUNT 1
const struct command g_disk_commands[DISK_COMMANDS_COUNT] = {
   { "dir", tdisk_dir }
};

static TASK_RETVAL trepl_disk( const struct astring* cli ) {
   const char* tok;
   uint8_t i = 0;

   tok = alpha_tok( cli, ' ', 1 );
   if( NULL == tok ) {
      return 1;
   }

   for( i = 0 ; DISK_COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp_cc(
         tok, CMD_MAX_LEN, g_disk_commands[i].command, CMD_MAX_LEN, ' '
      ) ) {
         return g_disk_commands[i].callback( cli );
      }
   }

   return RETVAL_BAD_ARGS;
}

#endif /* USE_DISK */

/**
 * Map typed commands to callbacks.
 */
#define COMMANDS_COUNT 3
static const struct command g_commands[COMMANDS_COUNT] = {
   { "sys", trepl_sys },
   { "net", trepl_net },
   { "dsk", trepl_disk }
};

TASK_RETVAL do_command( const struct astring* cli ) {
   uint8_t i = 0;

   for( i = 0 ; COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp_c( g_commands[i].command, CMD_MAX_LEN, cli, ' ' ) ) {
         return g_commands[i].callback( cli );
      }
   }

   return RETVAL_NOT_FOUND;
}

#endif /* USE_CONSOLE */

