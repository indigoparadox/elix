
#include "console.h"
#include "display.h"
#include "mem.h"

#ifdef CONSOLE_SERIAL
#else
#include "keyboard.h"
#endif /* CONSOLE_SERIAL */

/* Memory IDs for console tasks. */
#define REPL_MID_LINE 1
#define REPL_MID_CUR_POS 2

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

/* Try to save some stack. */
union tprintf_spec {
   int d;
   char c;
   uint8_t x;
   char* s;
};

void tprintf( const char* pattern, ... ) {
   va_list args;
   int i = 0;
   char last = '\0';
   int len = 0;
   union tprintf_spec spec;

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
               spec.s = va_arg( args, char* );
               tputs( spec.s );
               break;

            case 'd':
               spec.d = va_arg( args, int );
               while( 10 <= spec.d ) {
                  display_putc( (char)(spec.d % 10) );
                  spec.d /= 10;
               }
               break;

            case 'x':
               /* TODO: Hex */
               break;

            case 'c':
               spec.c = va_arg( args, int );
               display_putc( spec.c );
               break;
         }
      } else if( '%' != pattern[i] ) {
         /* Print non-escape characters verbatim. */
         display_putc( pattern[i] );
      }

      last = pattern[i];
   }
}

void trepl_init() {
}

void truncmd( char* line, int line_len ) {
   if( 0 == mstrcmp( line, "netr" ) ) {
      g_net_con_request = NET_REQ_RCVD;
   }
}

TASK_RETVAL trepl_task( TASK_PID pid ) {
   char c = '\0';
   uint8_t* cur_pos = NULL;
   int line_len = 0;
   char* line;

#ifdef CONSOLE_SERIAL
   if( 0 ) {
#else
   if( keyboard_hit() ) {
      c = keyboard_getc();
#endif /* CONSOLE_SERIAL */
      cur_pos = mget( pid, REPL_MID_CUR_POS, NULL );
      line = mget( pid, REPL_MID_LINE, &line_len );
      if( 0 == line_len ) {
         mset( pid, REPL_MID_CUR_POS, NULL, sizeof( uint8_t ) );
         mset( pid, REPL_MID_LINE, NULL, REPL_LINE_SIZE_MAX );
         cur_pos = mget( pid, REPL_MID_CUR_POS, NULL );
         line = mget( pid, REPL_MID_LINE, &line_len );
      }

      if( *cur_pos < REPL_LINE_SIZE_MAX ) {
         line[*cur_pos] = c;
         (*cur_pos)++;
         display_putc( c );

         truncmd( line, *cur_pos );
      } else {
         tputs( "INVALID COMMAND" );
         *cur_pos = 0;
         display_newline();
      }
   }

   return 0;
}

