
#include "code16.h"

#define CONSOLE_C
#include "console.h"
#include "display.h"
#include "mem.h"
#include "alpha.h"
#include "io.h"

#define TPRINT_PAD_ZERO 0
#define TPRINT_PAD_SPACE 1

void tputc( char c ) {
   /* ifdef for troubleshooting. Can be removed when console dev selection
    * works for sure.
    */
#ifdef CONSOLE_UART_WO
   uart_putc( 1, c );
#else
   g_io_output_cbs[g_console_out_dev_index]( g_console_out_dev_index, c );
#endif /* CONSOLE_UART_WO */
}

char tgetc() {
   /* ifdef for troubleshooting. Can be removed when console dev selection
    * works for sure.
    */
#ifdef CONSOLE_UART_WO
   return 0;
#else
   return 
      g_io_input_cbs[g_console_in_dev_index]( g_console_in_dev_index, false );
#endif /* CONSOLE_UART_WO */
}

char twaitc() {
   /* ifdef for troubleshooting. Can be removed when console dev selection
    * works for sure.
    */
#ifdef CONSOLE_UART_WO
   return 0;
#else
   return
      g_io_input_cbs[g_console_in_dev_index]( g_console_in_dev_index, true );
#endif /* CONSOLE_UART_WO */
}

void tprintf( const char* pattern, ... ) {
   va_list args;
   int i = 0, j = 0;
   char last = '\0';
   union mvalue spec;
   struct astring* astr_spec = NULL;
   struct astring* num_buffer = NULL;
   STRLEN_T padding = 0;
   char c;
   uint8_t pad_type = TPRINT_PAD_ZERO;

   alpha_astring( PID_MAIN, MID_PRINTF_NUMBUF, UTOA_DIGITS_MAX, NULL );

   va_start( args, pattern );

   for( i = 0 ; '\0' != pattern[i] ; i++ ) {
      c = pattern[i]; /* Separate so we can play tricks below. */
 
      if( '%' == last ) {
         /* Conversion specifier encountered. */
         mzero( &spec, sizeof( union mvalue ) );
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

