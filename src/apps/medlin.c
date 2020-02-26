
#include "../code16.h"

#include "repl.h"

#define MEDLIN_FLAG_EDIT 0x80

#define MEDLIN_MID_WORKPAD 1

const struct astring g_str_edln = astring_l( "edln" );

/* TODO: This is a suuuper hack. */
STRLEN_T last_len = 0;

static TASK_RETVAL medlin_handle_cmd( struct astring* cli ) {
   STRLEN_T i = 0;
   struct astring* workpad = NULL;
   
   workpad = alpha_astring(
      adhd_get_pid_by_gid( "edln" ), 
      MEDLIN_MID_WORKPAD,
      last_len,
      NULL );
   assert( NULL != workpad );

   for( i = 0 ; astring_len( cli ) > i ; i++ ) {
      switch( astring_char( cli, i ) ) {
      case 'q':
         repl_set_line_handler( NULL );
         adhd_kill_task( adhd_get_pid_by_gid( "edln" ) );
         break;

      case 'a':
         tprintf( "editing...\n" );
         repl_set_flag( MEDLIN_FLAG_EDIT );
         break;

      case 'l':
         tprintf( "%a\n", workpad );
         break;
      }
   }

   return RETVAL_OK;
}

static TASK_RETVAL medlin_edit_line( struct astring* cli ) {
   STRLEN_T i = 0;
   struct astring* workpad = NULL;

   workpad = alpha_astring(
      adhd_get_pid_by_gid( "edln" ), 
      MEDLIN_MID_WORKPAD,
      astring_len( cli ) + last_len,
      NULL );
   assert( NULL != workpad );

   tprintf( "len: %d\n", last_len );
   tprintf( "wpad: %a\n", workpad );
   tprintf( "cli: %a\n", cli );

   for( i = 0 ; astring_len( cli ) > i ; i++ ) {
      alpha_astring_append( workpad, astring_char( cli, i ) );
   }
   alpha_astring_append( workpad, '\n' );

   last_len = astring_len( workpad );

   repl_unset_flag( MEDLIN_FLAG_EDIT );

   return RETVAL_OK;
}

TASK_RETVAL medlin_line_handler( struct astring* cli ) {
   if( repl_flag( MEDLIN_FLAG_EDIT ) ) {
      tprintf( "editing from flag...\n" );
      return medlin_edit_line( cli );
   } else {
      return medlin_handle_cmd( cli );
   }
}

TASK_RETVAL medlin_task() {
   adhd_task_setup();

   adhd_yield();
   adhd_end_loop();
}

TASK_RETVAL medlin_launch( struct astring* cli ) {
   if( 0 == alpha_cmp( cli, &g_str_edln, ' ', false, true ) ) {
      tprintf( "starting edln...\n" );
      adhd_launch_task( medlin_task, "edln" );
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

