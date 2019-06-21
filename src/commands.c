
#ifdef USE_CONSOLE

#include "commands.h"

#include "net/net.h"
#include "console.h"

#define cmd_def( cmd, callback ) \
   { astring_l( cmd ), callback }

static struct command* next_command( const struct command* cmd_in ) {
   uint8_t* cmd_out = (uint8_t*)cmd_in;

   cmd_out += sizeof( struct command );
   cmd_out += ((struct astring*)cmd_out)->sz;
   cmd_out += sizeof( struct astring );

   return (struct command*)cmd_out;
}

/* = Command Callbacks */

extern struct astring g_str_netp;

static TASK_RETVAL tnet_start( const struct astring* cli ) {
   tprintf( "start?\n" );
   adhd_launch_task( net_respond_task );
   return RETVAL_OK;
}

static TASK_RETVAL tnet_rcvd( const struct astring* cli ) {
   int* received = NULL;
   TASK_PID pid;

   pid = adhd_get_pid_by_gid( &g_str_netp );
   received = mget( pid, NET_MID_RECEIVED, sizeof( int ) );
   tprintf( "frames rcvd: %d\n", *received );

   return RETVAL_OK;
}

#define NET_COMMANDS_COUNT 2
const struct command g_net_commands[NET_COMMANDS_COUNT] = {
   cmd_def( "rcvd", tnet_rcvd ),
   cmd_def( "start", tnet_start ),
};

static TASK_RETVAL trepl_net( const struct astring* cli ) {
   const char* tok;
   uint8_t i = 0;
   struct command* iter = (struct command*)g_net_commands;

   tok = alpha_tok( cli, ' ', 1 );
   if( NULL == tok ) {
      return 1;
   }

   for( i = 0 ; NET_COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp_c( tok, 10, &(iter->command), ' ' ) ) {
         return iter->callback( cli );
      } else {
         iter = next_command( iter );
      }
   }

   return RETVAL_BAD_ARGS;
}

static TASK_RETVAL trepl_sys( const struct astring* cli ) {

   return RETVAL_BAD_ARGS;
}

/**
 * Map typed commands to callbacks.
 */
#define COMMANDS_COUNT 2
static const struct command g_commands[COMMANDS_COUNT] = {
   cmd_def( "sys", trepl_sys ),
   cmd_def( "net", trepl_net ),
};

TASK_RETVAL do_command( const struct astring* cli ) {
   uint8_t i = 0;
   struct command* iter = (struct command*)g_commands;

   //i = alpha_cmp_l( cli, g_commands, COMMANDS_COUNT, ' ' );
   for( i = 0 ; COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp( cli, &(iter->command), ' ' ) ) {
         tprintf( "%d - %a - %d\n", trepl_net, &(iter->command), (uintptr_t)(iter->callback) );
         return iter->callback( cli );
      } else {
         iter = next_command( iter );
      }
   }

   return RETVAL_NOT_FOUND;
}

#endif /* USE_CONSOLE */

