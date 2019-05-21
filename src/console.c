
#define CONSOLE_C
#include "console.h"
#include "display.h"
#include "mem.h"
#include "kernel.h"
#include "stdlib.h"
#include "alpha.h"
#include "strings.h"

#ifdef CONSOLE_SERIAL
#else
#include "keyboard.h"
#endif /* CONSOLE_SERIAL */

/* Memory IDs for console tasks. */
#define REPL_MID_LINE 1
#define REPL_MID_CUR_POS 2

void tregcmd( struct repl_command* cmd ) {
   uint8_t idx = 0;

   while( 0 != g_repl_commands[idx].command->len ) {
      idx++;
      if( REPL_COMMANDS_MAX <= idx ) {
         /* No slots free! */
         return;
      }
   }

   mcopy( &(g_repl_commands[idx]), cmd, sizeof( struct repl_command ) );
}

void tputs( const struct astring* str ) {
#ifdef CONSOLE_SERIAL
#else
   STRLEN_T i = 0;
   for( i = 0 ; str->len > i ; i++ ) {
      display_putc( str->data[i] );
   }
#endif /* CONSOLE_SERIAL */
}

/* Try to save some stack. */
union tprintf_spec {
   int d;
   char c;
   uint8_t x;
   struct astring* s;
};

void tprintf( const struct astring* pattern, ... ) {
   va_list args;
   int i = 0;
   char last = '\0';
   union tprintf_spec spec;
   uint8_t num_buffer[sizeof( struct astring ) + INT_DIGITS_MAX] = { 0 };
   STRLEN_T padding = 0;
   char c;
#ifdef DEBUG
   int check = 0;
#endif /* DEBUG */

   /* Make sure the num_buffer knows how much space is available. */
   ((struct astring*)&num_buffer)->sz = INT_DIGITS_MAX;

   va_start( args, pattern );

   for( i = 0 ; pattern->len > i ; i++ ) {
      c = pattern->data[i]; /* Separate so we can play tricks below. */
      if( '\0' == pattern->data[i] ) {
         break; /* Early. */
      }
 
      if( '%' == last ) {
         /* Conversion specifier encountered. */
         switch( pattern->data[i] ) {
            case 's':
               spec.s = va_arg( args, struct astring* );
               tputs( spec.s );
               break;

            case 'd':
               spec.d = va_arg( args, int );
#ifdef DEBUG
               check = alpha_utoa(
#else
               alpha_utoa(
#endif /* DEBUG */
                  spec.d, (struct astring*)&num_buffer, 0, padding, 10 );
#ifdef DEBUG
               //assert( check == alpha_udigits(
#endif /* DEBUG */
               tputs( (struct astring*)&num_buffer );
               padding = 0; /* Reset. */
               break;

            case 'x':
               spec.d = va_arg( args, int );
#ifdef DEBUG
               check = alpha_utoa(
#else
               alpha_utoa(
#endif /* DEBUG */
                  spec.d, (struct astring*)&num_buffer, 0, padding, 16 );
               tputs( (struct astring*)&num_buffer );
               padding = 0; /* Reset. */
               break;

            case 'c':
               spec.c = va_arg( args, int );
               display_putc( spec.c );
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
         display_putc( c );
      }

      last = c;
   }
}

/* TODO: Move net command to net module. */
/*
void truncmd( char* line, int line_len ) {
   if( 0 == mcompare( line, "netr", 4 ) ) {
      g_net_con_request = NET_REQ_RCVD;
   } else if( 0 == mcompare( line, "quit", 4 ) ) {
      g_system_state = SYSTEM_SHUTDOWN;
   }
}
*/

TASK_RETVAL trepl_task( TASK_PID pid ) {
   char c = '\0';
   struct astring* line;

   mprint();

#ifdef CONSOLE_SERIAL
   if( 0 ) {
#else
   if( !keyboard_hit() ) {
      return 0;
   }

   c = keyboard_getc();
#endif /* CONSOLE_SERIAL */
   /* Dynamically allocate the line buffer so we can clear it from memory
    * during other programs.
    */
   line = 
      mget( pid, REPL_MID_LINE, NULL, astring_sizeof( REPL_LINE_SIZE_MAX ) );

   if( line->len + 1 >= line->sz ) {
      /* Line would be too long if we accepted this char. */
      tputs( &g_str_invalid );
      astring_clear( line );
      display_newline();
      return 0;
   }

   switch( c ) {
      case '\n':
         //truncmd( line, cur_pos );
         astring_clear( line );
         break;

      default:
         astring_append( line, c );
#ifdef CONSOLE_SERIAL
#else
         display_putc( c );
#endif /* CONSOLE_SERIAL */
         break;
   }

   return 0;
}

