
#include "console.h"
#include "display.h"

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

