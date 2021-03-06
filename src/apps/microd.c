
#include "../code16.h"
#include "../console.h"
#include "../adhd.h"

TASK_RETVAL microd_task() {
   adhd_task_setup();

   adhd_yield();

   adhd_end_loop();
}

TASK_RETVAL microd_launch( struct astring* cli ) {
   if( 0 == alpha_cmp_c( "microd", 5, cli, ' ', false, true ) ) {
      tprintf( "starting...\n" );
      adhd_launch_task( microd_task, "micd" );
      /* adhd_kill_task( repl_pid ); */
      return RETVAL_OK;
   }

   return RETVAL_NOT_FOUND;
}

__attribute__( (constructor) )
static void microd_register() {
   g_console_apps[g_console_apps_top++] = microd_launch;
}

