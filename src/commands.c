
#ifdef USE_CONSOLE

#include "commands.h"

#include "net/net.h"
#include "console.h"

#define cmd_def( cmd, callback ) \
   { astring_l( cmd ), callback }

/* = Command Callbacks */

#define NET_COMMANDS_COUNT 2
const struct astring net_commands[NET_COMMANDS_COUNT] = {
   astring_l( "rcvd" ),
   astring_l( "start" ),
};

static TASK_RETVAL trepl_net( const struct astring* cli ) {
   const char* tok;
   int* received = NULL;
   TASK_PID pid;

   pid = adhd_get_pid_by_gid( "netp" );
   tok = alpha_tok( cli, ' ', 1 );

   if( NULL == tok ) {
      return 1;
   }

   /* Only compare the first five chars. */
   switch( alpha_cmp_cl( tok, 5, net_commands, NET_COMMANDS_COUNT, ' ' ) ) {
   case 0:
      received = mget( pid, NET_MID_RECEIVED, sizeof( int ) );
      tprintf( "frames rcvd: %d\n", *received );
      return 0;

   case 1:
      adhd_launch_task( net_respond_task );
      return 0;
   }

   return 1;
}

static TASK_RETVAL trepl_sys( const struct astring* cli ) {

   return 0;
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

   //i = alpha_cmp_l( cli, g_commands, COMMANDS_COUNT, ' ' );
   for( i = 0 ; COMMANDS_COUNT > i ; i++ ) {
      if( 0 == alpha_cmp( cli, &(g_commands[i].command), ' ' ) ) {
         return g_commands[i].callback( cli );
      }
   }

   return 1;
}

#endif /* USE_CONSOLE */

