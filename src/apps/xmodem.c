
#include "../code16.h"
#include "../console.h"
#include "../adhd.h"

const struct astring g_str_xmdm = astring_l( "xmdm" );

TASK_RETVAL xmodem_task() {
   adhd_task_setup();

   adhd_set_gid( &g_str_xmdm );
   
   adhd_yield();

   adhd_end_loop();
}

TASK_RETVAL xmodem_launch( const struct astring* cli ) {
   tprintf( "trying xmodem: %a / %d / %d\n", cli, g_str_xmdm.len, alpha_wordlen( cli ) );
   if( 0 == alpha_cmp( cli, &g_str_xmdm, ' ', false, true ) ) {
      tprintf( "starting...\n" );
      adhd_launch_task( xmodem_task );
      return RETVAL_OK;
   }

   return RETVAL_NOT_FOUND;
}

__attribute__( (constructor) )
static void xmodem_register() {
   g_console_apps[g_console_apps_top++] = xmodem_launch;
}

