
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
   "   .`_   _`.   ",
   "  / / | | \\ \\  ",
   " | |  | |  | | ",
   " | |  | |  | | ",
   "  \\ \\_| |_/ /  ",
   "   `._`\\ \\.'   ",
   "        \\_\\    "
};

uint8_t g_console_flags = 0;

const struct astring g_str_ready = astring_l( "ready\n" );

/* Memory IDs for console tasks. */
#define REPL_MID_LINE      1
#define REPL_MID_CUR_POS   2
#define REPL_MID_ARG_MIN   10
/* Empty */
#define REPL_MID_ARG_MAX   20

/* Try to save some stack. */
union tprintf_spec {
   int d;
   char c;
   uint8_t x;
   struct astring* a;
   char* s;
};

void tprintf( const char* pattern, ... ) {
   va_list args;
   int i = 0, j = 0;
   char last = '\0';
   union tprintf_spec spec;
   uint8_t num_buffer[sizeof( struct astring ) + INT_DIGITS_MAX] = { 0 };
   struct astring* buffer_ptr = (struct astring*)&num_buffer;
   STRLEN_T padding = 0;
   char c;

   /* Make sure the num_buffer knows how much space is available. */
   buffer_ptr->sz = INT_DIGITS_MAX;

   va_start( args, pattern );

   for( i = 0 ; '\0' != pattern[i] ; i++ ) {
      c = pattern[i]; /* Separate so we can play tricks below. */
 
      if( '%' == last ) {
         /* Conversion specifier encountered. */
         switch( pattern[i] ) {
            case 'a':
               spec.a = va_arg( args, struct astring* );
               j = 0;
               while( '\0' != spec.a->data[j] && spec.a->len > j ) {
                  tputc( spec.a->data[j++] );
               }
               break;

            case 's':
               spec.s = va_arg( args, char* );
               j = 0;
               while( '\0' != spec.s[j] ) {
                  tputc( spec.s[j++] );
               }
               break;

            case 'd':
               spec.d = va_arg( args, int );
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
               /* TODO: Handle multi-digit qty padding. */
               padding = c - '0';
               c = '%';
               break;
         }
      } else if( '%' != c ) {
         /* Print non-escape characters verbatim. */
         tputc( c );
      }

      last = c;
   }
}

TASK_RETVAL trepl_task() {
   char c = '\0';
   struct astring* line;
   //struct CHIIPY_TOKEN* token;
   //struct astring* arg;
   uint8_t i = 0;
   uint8_t retval = 0;

   adhd_task_setup();

   if( !(g_console_flags & CONSOLE_FLAG_INITIALIZED) ) {
      for( i = 0 ; 8 > i ; i++ ) {
         tprintf( qd_logo[i] );
         tputc( '\n' );
      }
      tprintf( "QD console v" VERSION "\n" );
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
      adhd_get_pid(), REPL_MID_LINE, REPL_LINE_SIZE_MAX + 1 );
   //token = mget( pid, REPL_MID_LINE, 30 );

   if(
      line->len + 1 >= line->sz ||
      (('\r' == c || '\n' == c) && 0 == line->len)
   ) {
      /* Line would be too long if we accepted this char. */
      display_newline( g_console_dev_index );
      tputs( &g_str_invalid );
      astring_clear( line );
      adhd_yield();
      adhd_continue_loop();
   }

   switch( c ) {
      case '\r':
      case '\n':
         display_newline( g_console_dev_index );
         retval = do_command( line );
         if( RETVAL_NOT_FOUND == retval ) {
            tputs( &g_str_invalid );
         } else if( RETVAL_BAD_ARGS == retval ) {
            tprintf( "bad arguments\n" );
         } else {
            tputs( &g_str_ready );
         }
         astring_clear( line );
         break;

      default:
         //chiipy_lex_tok( c, token );
         astring_append( line, c );
         tputc( c );
         break;
   }

   adhd_yield();
   adhd_end_loop();
}

#endif /* USE_CONSOLE */

