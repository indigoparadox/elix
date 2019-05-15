
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

void tputsl( const char* str, int len, uint8_t fg, uint8_t bg ) {
#ifdef CONSOLE_COLOR
   /* Set the colors, show the message, then set them back. */
   display_set_colors( fg, bg );
#endif /* CONSOLE_COLOR */
   tputs( str, len );
#ifdef CONSOLE_COLOR
   display_set_colors( g_term_fg, g_term_bg );
#endif /* CONSOLE_COLOR */
}

void tputs( const char* str, int len ) {
#ifdef CONSOLE_SERIAL
#else
   display_puts( str, len );
#endif /* CONSOLE_SERIAL */
}

void tprintf( const char* pattern, int len, ... ) {
   va_list args;
   va_start( args, len );
   int i = 0;
   char last = '\0';
   char* s = NULL;
   int s_len = 0;

   for( i = 0 ; len > i ; i++ ) {
      if( '\0' == pattern[i] ) {
         break; /* Early. */
      }
 
      if( '%' == last ) {
         /* Conversion specifier encountered. */
         switch( pattern[i] ) {
            case 's':
               /* TODO: Make sure a length was given! */
               s = va_arg( args, char* );
               s_len = va_arg( args, int );
               tputs( s, s_len );
               break;
         }
      } else {
         display_putc( pattern[i] );
      }

      last = pattern[i];
   }
}

