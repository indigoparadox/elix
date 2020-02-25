
#include "../code16.h"

#include "repl.h"

const struct astring g_str_edln = astring_l( "edln" );

TASK_RETVAL medlin_line_handler( struct astring* cli ) {
    if(
      0 == alpha_cmp_c( "exit", 4, cli, '\0', false, 3 )
    ) {
      repl_set_line_handler( NULL );
    }
   return RETVAL_OK;
}

TASK_RETVAL medlin_task() {
   adhd_task_setup();

   adhd_set_gid( "edln" );

   adhd_yield();
   adhd_end_loop();
}

TASK_RETVAL medlin_launch( struct astring* cli ) {
   if( 0 == alpha_cmp( cli, &g_str_edln, ' ', false, true ) ) {
      tprintf( "starting edln...\n" );
      adhd_launch_task( medlin_task );
      repl_set_line_handler( medlin_line_handler );
      /* adhd_kill_task( repl_pid ); */
      return RETVAL_OK;
   }

   return RETVAL_NOT_FOUND;
}

__attribute__( (constructor) )
static void medlin_register() {
   g_console_apps[g_console_apps_top++] = medlin_launch;
}

