
#include "../code16.h"
#include "../console.h"
#include "../adhd.h"

TASK_RETVAL xmodem_task() {
   adhd_task_setup();

   adhd_yield();

   adhd_end_loop();
}

TASK_RETVAL xmodem_launch( struct astring* cli ) {
   if( 0 == alpha_cmp_c( "xmodem", 6, cli, ' ', false, true ) ) {
      tprintf( "starting...\n" );
      adhd_launch_task( xmodem_task, "xmdm" );
      return RETVAL_OK;
   }

   return RETVAL_NOT_FOUND;
}

__attribute__( (constructor) )
static void xmodem_register() {
   g_console_apps[g_console_apps_top++] = xmodem_launch;
}

