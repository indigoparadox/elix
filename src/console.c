
#include "code16.h"

#define CONSOLE_C
#include "console.h"
#include "display.h"
#include "mem.h"
#include "alpha.h"
#include "io.h"

#define TPRINT_PAD_ZERO 0
#define TPRINT_PAD_SPACE 1

void abort() {
}

static void tpad( char pad, STRLEN_T len ) {
   uint8_t i = 0;

   if( 0 >= len || '\0' == pad ) {
      return;
   }

   while( 0 == len || i < len) {
      tputc( pad );
      i++;
   }
}

void tprintf( const char* pattern, ... ) {
   va_list args;
   int i = 0, j = 0;
   char last = '\0';
   union mvalue spec;
   struct astring* astr_spec = NULL;
   char buffer[UTOA_DIGITS_MAX + 1];
   STRLEN_T pad_len = 0;
   char c;
   uint8_t pad_char = ' ';

   va_start( args, pattern );

   for( i = 0 ; '\0' != pattern[i] ; i++ ) {
      c = pattern[i]; /* Separate so we can play tricks below. */
 
      if( '%' == last ) {
         /* Conversion specifier encountered. */
         mzero( &spec, sizeof( union mvalue ) );
         mzero( buffer, UTOA_DIGITS_MAX + 1 );
         switch( pattern[i] ) {
            case 'a':
               astr_spec = va_arg( args, struct astring* );

               /* Print padding. */
               pad_len -= astr_spec->len;
               tpad( pad_char, pad_len );

               /* Print string. */
               while( '\0' != astr_spec->data[j] && astr_spec->len > j ) {
                  tputc( astr_spec->data[j++] );
               }
               break;

            case 's':
               spec.s = va_arg( args, char* );

               /* Print padding. */
               pad_len -= alpha_strlen_c( spec.s, STRLEN_MAX );
               tpad( pad_char, pad_len );

               /* Print string. */
               j = 0;
               while( '\0' != spec.s[j] ) {
                  tputc( spec.s[j++] );
               }
               break;

            case 'd':
               spec.d = va_arg( args, UTOA_T );
               
               /* Print padding. */
               pad_len -= alpha_udigits( spec.d, 10 );
               tpad( pad_char, pad_len );

               /* Print number. */
               if(
                  0 < alpha_utoa_c( spec.d, buffer, UTOA_DIGITS_MAX + 1, 10 )
               ) {
                  j = 0;
                  while( '\0' != buffer[j] && j <= UTOA_DIGITS_MAX ) {
                     tputc( buffer[j] );
                     j++;
                  }
               }
               break;

            case 'x':
               spec.d = va_arg( args, int );

               /* Print padding. */
               pad_len -= alpha_udigits( spec.d, 16 );
               tpad( pad_char, pad_len );

               /* Print number. */
               if(
                  0 < alpha_utoa_c( spec.d, buffer, UTOA_DIGITS_MAX + 1, 16 )
               ) {
                  tputc( '0' );
                  tputc( 'x' );
                  j = 0;
                  while( '\0' != buffer[j] && j <= UTOA_DIGITS_MAX ) {
                     tputc( buffer[j] );
                     j++;
                  }
               }
               break;

            case 'c':
               spec.c = va_arg( args, int );

               /* Print padding. */
               tpad( pad_char, pad_len );

               /* Print char. */
               tputc( spec.c );
               break;

#ifndef CONSOLE_NO_PRINTF_PTR
            case 'p':
               spec.p = va_arg( args, void* );

               /* Print padding. */
               pad_len -= alpha_udigits( (uintptr_t)spec.p, 16 );
               tpad( pad_char, pad_len );

               /* Print pointer as number. */
               if( 0 <
                  alpha_utoa_c(
                     (uintptr_t)spec.p, buffer, UTOA_DIGITS_MAX + 1, 16 )
               ) {
                  j = 0;
                  while( '\0' != buffer[j] && j <= UTOA_DIGITS_MAX ) {
                     tputc( buffer[j] );
                     j++;
                  }
               }
               break;
#endif /* !CONSOLE_NO_PRINTF_PTR */

            case '%':
               last = '\0';
               tputc( '%' );
               break;

            case '0':
               /* If we haven't started counting padding with a non-zero number,
                * this must be a 0-padding signifier.
                */
               if( 0 >= pad_len ) {
                  pad_char = '0';
                  c = '%';
                  break;
               }
               /* If we've already started parsing padding count digits, then
                * fall through below as a regular number.
                */
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
               pad_len *= 10;
               pad_len += (c - '0'); /* Convert from char to int. */
               c = '%';
               break;
         }
      } else if( '%' != c ) {
         pad_char = ' '; /* Reset padding. */
         pad_len = 0; /* Reset padding. */
         /* Print non-escape characters verbatim. */
         switch( c ) {
         case '\n':
#if CONSOLE_NEWLINE_R_N == CONSOLE_NEWLINE
            tputc( '\r' );
#endif /* CONSOLE_NEWLINE */
            tputc( '\n' );
            break;

         default:
            tputc( c );
            break;
         }
      }

      last = c;
   }
}

