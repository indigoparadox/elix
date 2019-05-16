
#include "console.h"
#include "display.h"
#include "mem.h"

#ifdef CONSOLE_SERIAL
#else
#include "keyboard.h"
#endif /* CONSOLE_SERIAL */

#define REPL_LINE_SIZE_MAX 20

/* Memory IDs for console tasks. */
#define REPL_MID_LINE 1
#define REPL_MID_CUR_POS 1

#ifdef CONSOLE_COLOR
static uint8_t g_term_bg;
static uint8_t g_term_fg;
#endif /* CONSOLE_COLOR */

void tsetsl( uint8_t fg, uint8_t bg ) {
#ifdef CONSOLE_COLOR
   /* TODO: Use a short and a single op to save on instructions. */
   g_term_bg = bg;
   g_term_fg = fg;
#endif /* CONSOLE_COLOR */
}

void tputsl( const char* str, uint8_t fg, uint8_t bg ) {
#ifdef CONSOLE_COLOR
   /* Set the colors, show the message, then set them back. */
   display_set_colors( fg, bg );
#endif /* CONSOLE_COLOR */
   tputs( str );
#ifdef CONSOLE_COLOR
   display_set_colors( g_term_fg, g_term_bg );
#endif /* CONSOLE_COLOR */
}

void tputs( const char* str ) {
#ifdef CONSOLE_SERIAL
#else
   display_puts( str );
#endif /* CONSOLE_SERIAL */
}

char tgetc() {
#ifdef CONSOLE_SERIAL
#else
   return keyboard_getc();
#endif /* CONSOLE_SERIAL */
}

void tprintf( const char* pattern, ... ) {
   va_list args;
   int i = 0;
   char last = '\0';
   char* s = NULL;
   int len = 0;

   va_start( args, pattern );
   len = mstrlen( pattern );

   for( i = 0 ; len > i ; i++ ) {
      if( '\0' == pattern[i] ) {
         break; /* Early. */
      }
 
      if( '%' == last ) {
         /* Conversion specifier encountered. */
         switch( pattern[i] ) {
            case 's':
               s = va_arg( args, char* );
               tputs( s );
               break;
         }
      } else {
         display_putc( pattern[i] );
      }

      last = pattern[i];
   }
}

void trepl_init() {
}

int trepl_task( int pid ) {
   char c = '\0';
   uint8_t cur_pos = 0;
   int line_len = 0;
   char* line;

   if( keyboard_hit() ) {
      c = keyboard_getc();
      cur_pos = mget_int( pid, REPL_MID_CUR_POS );
      line = mget( pid, REPL_MID_LINE, &line_len );
      if( 0 == line_len ) {
         mset( pid, REPL_MID_LINE, NULL, REPL_LINE_SIZE_MAX );
         line = mget( pid, REPL_MID_LINE, &line_len );
      }

      //printf( "%c\n", c );
      printf( "%p\n", line );
      printf( "%d: %s\n", cur_pos, line );
      mprint();

      line[cur_pos] = c;
      cur_pos++;
      tputs( line );

      mset( pid, REPL_MID_CUR_POS, &cur_pos, sizeof( int ) );
      mset( pid, REPL_MID_LINE, line, REPL_LINE_SIZE_MAX );
   }

   return 0;
}

