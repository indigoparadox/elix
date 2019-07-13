
#include "../code16.h"
#include "../console.h"
#include "../adhd.h"

const struct astring g_str_micd = astring_l( "micd" );

TASK_RETVAL microd_task() {
   adhd_task_setup();

   adhd_set_gid( &g_str_micd );
   
   adhd_yield();

   adhd_end_loop();
}

TASK_RETVAL microd_launch( const struct astring* cli ) {
   tprintf( "trying microd: %a / %d / %d\n", cli, g_str_micd.len, alpha_wordlen( cli ) );
   if( 0 == alpha_cmp( cli, &g_str_micd, ' ', false, true ) ) {
      tprintf( "starting...\n" );
      adhd_launch_task( microd_task );
      return RETVAL_OK;
   }

   return RETVAL_NOT_FOUND;
}

__attribute__( (constructor) )
static void microd_register() {
   g_console_apps[g_console_apps_top++] = microd_launch;
}

