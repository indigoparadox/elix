
#ifdef USE_CONSOLE

#define CONSOLE_C
#include "console.h"
#include "display.h"
#include "mem.h"
#include "kernel.h"
#include "stdlib.h"
#include "alpha.h"
#include "strings.h"
#include "net/net.h"
#include "commands.h"

#include <chiipy.h>

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

uint8_t g_console_flags = 0;

const struct astring g_str_ready = astring_l( "ready" CONSOLE_NEWLINE );

#define TPRINT_PAD_ZERO 0
#define TPRINT_PAD_SPACE 1

void tprintf( const char* pattern, ... ) {
   va_list args;
   int i = 0, j = 0;
   char last = '\0';
   union mvalue spec;
   struct astring* astr_spec = NULL;
   uint8_t num_buffer[sizeof( struct astring ) + UINT32_DIGITS_MAX] = { 0 };
   struct astring* buffer_ptr = (struct astring*)&num_buffer;
   STRLEN_T padding = 0;
   char c;
   uint8_t pad_type = TPRINT_PAD_ZERO;

   /* Make sure the num_buffer knows how much space is available. */
   buffer_ptr->sz = UINT32_DIGITS_MAX;

   va_start( args, pattern );

   for( i = 0 ; '\0' != pattern[i] ; i++ ) {
      c = pattern[i]; /* Separate so we can play tricks below. */
 
      if( '%' == last ) {
         /* Conversion specifier encountered. */
         switch( pattern[i] ) {
            case 'a':
               astr_spec = va_arg( args, struct astring* );
               j = 0;
               while( '\0' != astr_spec->data[j] && astr_spec->len > j ) {
                  tputc( astr_spec->data[j++] );
               }
               break;

            case 's':
               spec.s = va_arg( args, char* );
               j = 0;
               while(
                  '\0' != spec.s[j] &&          /* NULL found, or... */
                  (0 == padding || j < padding) /* Padding present. */
               ) {
                  tputc( spec.s[j++] );
               }
               if( TPRINT_PAD_SPACE == pad_type ) {
                  while( 0 == padding || j < padding ) {
                     tputc( ' ' );
                     j++;
                  }
               }
               break;

            case 'd':
               spec.d = va_arg( args, UTOA_T );
               alpha_utoa(
                  spec.d, (struct astring*)&num_buffer, 0, padding, 10 );
               tputs( (struct astring*)&num_buffer );
               padding = 0; /* Reset. */
               break;

            case 'x':
               spec.d = va_arg( args, int );
               alpha_utoa(
                  spec.d, (struct astring*)&num_buffer, 0, padding, 16 );
               tputs( (struct astring*)&num_buffer );
               padding = 0; /* Reset. */
               break;

            case 'c':
               spec.c = va_arg( args, int );
               tputc( spec.c );
               break;

            case ' ':
               pad_type = TPRINT_PAD_SPACE;
               c = '%';
               break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
               /* Handle multi-digit qty padding. */
               padding *= 10;
               padding += (c - '0'); /* Convert from char to int. */
               c = '%';
               break;
         }
      } else if( '%' != c ) {
         pad_type = TPRINT_PAD_ZERO; /* Reset padding. */
         padding = 0; /* Reset padding. */
         /* Print non-escape characters verbatim. */
         tputc( c );
      }

      last = c;
   }
}

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
      tputs( &g_str_ready );
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
      tputs( &g_str_invalid );
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
            tputs( &g_str_invalid );
         } else if( RETVAL_BAD_ARGS == retval ) {
            tprintf( "bad arguments" CONSOLE_NEWLINE );
         } else {
            tputs( &g_str_ready );
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

#endif /* USE_CONSOLE */

