
#include "../code16.h"

#include "repl.h"

const struct astring g_str_edln = astring_l( "edln" );

TASK_RETVAL medlin_task() {
   char c = '\0';

   adhd_task_setup();

   adhd_set_gid( "edln" );

   if( !twaitc() ) {
      adhd_yield();
   }

   c = tgetc();

   switch( c ) {
   default:
      tprintf( "%c", c );
   }
   
   adhd_yield();
   adhd_end_loop();
}

TASK_RETVAL medlin_launch( struct astring* cli, TASK_PID repl_pid ) {
   tprintf( "trying medlin: %a / %d / %d\n", cli, g_str_edln.len, alpha_wordlen( cli ) );
   if( 0 == alpha_cmp( cli, &g_str_edln, ' ', false, true ) ) {
      tprintf( "killing repl (%d) and starting...\n", repl_pid );
      adhd_launch_task( medlin_task );
      adhd_kill_task( repl_pid );
      return RETVAL_OK;
   }

   return RETVAL_NOT_FOUND;
}

__attribute__( (constructor) )
static void medlin_register() {
   g_console_apps[g_console_apps_top++] = medlin_launch;
}

