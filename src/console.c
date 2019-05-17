
#include "console.h"
#include "display.h"
#include "mem.h"
#include "kernel.h"
#include "stdlib.h"
#include "alpha.h"

#ifdef CONSOLE_SERIAL
#else
#include "keyboard.h"
#endif /* CONSOLE_SERIAL */

/* Memory IDs for console tasks. */
#define REPL_MID_LINE 1
#define REPL_MID_CUR_POS 2

static STRLEN_T cur_pos = 0;

void tregcmd( struct repl_command cmd* ) {
   uint8_t idx = 0;

   while( '\0' != g_repl_commands[idx].command[0] ) {
      idx++;
      if( REPL_COMMANDS_MAX <= idx ) {
         /* No slots free! */
         return;
      }
   }

   mcopy( &(g_repl_commands[idx]), cmd, sizeof( struct repl_command ) );
}

void tputs( const char* str, STRLEN_T len_max ) {
#ifdef CONSOLE_SERIAL
#else
   int i = 0;
   int len = 0;
   
   len = alpha_strlen( str, len_max );
   for( i = 0 ; len > i ; i++ ) {
      display_putc( str[i] );
   }
#endif /* CONSOLE_SERIAL */
}

/* Try to save some stack. */
union tprintf_spec {
   int d;
   char c;
   uint8_t x;
   char* s;
};

void tprintf( const char* pattern, STRLEN_T pattern_len, ... ) {
   va_list args;
   int i = 0;
   char last = '\0';
   int len = 0;
   union tprintf_spec spec;

   va_start( args, pattern_len );
   len = alpha_strlen( pattern, pattern_len );

   for( i = 0 ; len > i ; i++ ) {
      if( '\0' == pattern[i] ) {
         break; /* Early. */
      }
 
      if( '%' == last ) {
         /* Conversion specifier encountered. */
         switch( pattern[i] ) {
            case 's':
               spec.s = va_arg( args, char* );
               tputs( spec.s, 255 ); /* XXX: Store and retrieve strlen. */
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
   int line_len = 0;
   char* line;

#ifdef CONSOLE_SERIAL
   if( 0 ) {
#else
   if( keyboard_hit() ) {
      c = keyboard_getc();
#endif /* CONSOLE_SERIAL */
      line = mget( pid, REPL_MID_LINE, &line_len );
      if( 0 == line_len ) {
         mset( pid, REPL_MID_CUR_POS, NULL, sizeof( uint8_t ) );
         mset( pid, REPL_MID_LINE, NULL, REPL_LINE_SIZE_MAX );
         line = mget( pid, REPL_MID_LINE, &line_len );
      }

      if( cur_pos < REPL_LINE_SIZE_MAX ) {
         switch( c ) {
            case '\n':
               //truncmd( line, cur_pos );
               mzero( line, REPL_LINE_SIZE_MAX );
               break;

            default:
               line[cur_pos] = c;
               cur_pos++;
#ifdef CONSOLE_SERIAL
#else
               display_putc( c );
#endif /* CONSOLE_SERIAL */
               break;
         }
      } else {
         tputs( "INVALID COMMAND" );
         cur_pos = 0;
         mzero( line, REPL_LINE_SIZE_MAX );
         display_newline();
      }
   }

   return 0;
}

