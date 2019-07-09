
#include "../console.h"
#include "../commands.h"

#define CONSOLE_FLAG_INITIALIZED    0x01

#define VERSION "2019.21"

/* Memory IDs for console tasks. */
#define REPL_MID_LINE      1
#define REPL_MID_CUR_POS   2
#define REPL_MID_ARG_MIN   10
/* Empty */
#define REPL_MID_ARG_MAX   20

#define REPL_LINE_SIZE_MAX 20

uint8_t g_console_flags = 0;

__attribute__( (constructor) )
void repl_constructor() {
   adhd_launch_task( trepl_task );
}

const char qd_logo[8][16] = {
   "     _____     ",
   "   .`_| |_`.   ",
   "  / /_| |_\\ \\  ",
   " |  __| |___|  ",
   " | |  | |      ",
   "  \\ \\_| |___   ",
   "   `._|_____/  ",
   "               "
};

TASK_RETVAL trepl_task() {
   char c = '\0';
   const struct astring* line;
   //struct CHIIPY_TOKEN* token;
   //struct astring* arg;
   uint8_t i = 0;
   uint8_t retval = 0;

   adhd_task_setup();

   if( !(g_console_flags & CONSOLE_FLAG_INITIALIZED) ) {
      for( i = 0 ; 8 > i ; i++ ) {
         tprintf( qd_logo[i] );
         tprintf( CONSOLE_NEWLINE );
      }
      tprintf( "EL console v" VERSION CONSOLE_NEWLINE );
      tprintf( "ptr %d bytes" CONSOLE_NEWLINE, sizeof( void* ) );
      tprintf( "ready" CONSOLE_NEWLINE );
      g_console_flags |= CONSOLE_FLAG_INITIALIZED;
   }

   if( !twaitc() ) {
      adhd_yield();
   }

   c = tgetc();
   /* Dynamically allocate the line buffer so we can clear it from memory
    * during other programs. Add +1 so there's always a NULL.
    */
   line = alpha_astring(
      adhd_get_pid(), REPL_MID_LINE, REPL_LINE_SIZE_MAX + 1, NULL );

   if(
      line->len + 1 >= line->sz ||
      (('\r' == c || '\n' == c) && 0 == line->len)
   ) {
      /* Line would be too long if we accepted this char. */
      tprintf( CONSOLE_NEWLINE );
      tprintf( "invalid" CONSOLE_NEWLINE );
      alpha_astring_clear( adhd_get_pid(), REPL_MID_LINE );
      adhd_yield();
      adhd_continue_loop();
   }

   switch( c ) {
      case '\r':
      case '\n':
         tprintf( CONSOLE_NEWLINE );
         retval = do_command( line );
         if( RETVAL_NOT_FOUND == retval ) {
            tprintf( "invalid" CONSOLE_NEWLINE );
         } else if( RETVAL_BAD_ARGS == retval ) {
            tprintf( "bad arguments" CONSOLE_NEWLINE );
         } else {
            tprintf( "ready" CONSOLE_NEWLINE );
         }
         alpha_astring_clear( adhd_get_pid(), REPL_MID_LINE );
         break;

      default:
         //chiipy_lex_tok( c, token );
         alpha_astring_append( adhd_get_pid(), REPL_MID_LINE, c );
         tputc( c );
         break;
   }

   adhd_yield();
   adhd_end_loop();
}

