
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

#include <chiipy.h>

#ifdef CONSOLE_SERIAL
#else
#include "keyboard.h"
#endif /* CONSOLE_SERIAL */

int trepl_service( char* cli ) {
   //const char* svc;

   //svc = alpha_tok( cli, 1, ' ' );

   return 0;
}

/* === Command Registration === */

struct command {
   struct astring command;
   int (*callback)( char* cli );
};

#define cmd_def( cmd, callback ) \
   { astring_l( cmd ), callback }

static const struct command g_commands[] = {
   cmd_def( "service", trepl_service ),
   cmd_def( "", NULL )
};

/* === Console Functions === */

/* Memory IDs for console tasks. */
#define REPL_MID_LINE      1
#define REPL_MID_CUR_POS   2
#define REPL_MID_ARG_MIN   10
/* Empty */
#define REPL_MID_ARG_MAX   20

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

void tprintf( const char* pattern, ... ) {
   va_list args;
   int i = 0;
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
            case 's':
               spec.s = va_arg( args, struct astring* );
               tputs( spec.s );
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

#include <stdio.h>
TASK_RETVAL trepl_task() {
   char c = '\0';
   struct astring* line;
   //struct CHIIPY_TOKEN* token;
   //struct astring* arg;
   //uint8_t i = 0;

   adhd_task_setup();

#ifdef CONSOLE_SERIAL
   //if( 0 ) {
#else
   if( !keyboard_hit() ) {
      adhd_yield();
   }

   c = keyboard_getc();
#endif /* CONSOLE_SERIAL */
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
      display_newline();
      tputs( &g_str_invalid );
      astring_clear( line );
      adhd_yield();
      adhd_continue_loop();
   }

   switch( c ) {
      case '\r':
      case '\n':
         display_newline();
         if( 0 == alpha_cmp_c( "exit", line, '\n' ) ) {
            g_system_state = SYSTEM_SHUTDOWN;
         } else if( 0 == alpha_cmp_c( "net", line, '\n' ) ) {
            adhd_launch_task( net_respond_task );
         } else {
            tputs( &g_str_invalid );
         }
         astring_clear( line );
         break;

      case ' ':
         /*i = REPL_MID_ARG_MIN;
         do {
            arg = mget( pid, i
         cstack_push( */
         break;

      default:
         //chiipy_lex_tok( c, token );
         astring_append( line, c );
#ifdef CONSOLE_SERIAL
#else
         /* display_putc( c ); */
         tprintf( "%c", c );
#endif /* CONSOLE_SERIAL */
         break;
   }

   adhd_yield();
   adhd_end_loop();
}

#endif /* USE_CONSOLE */

