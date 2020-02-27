
#include "cstd.h"

#include "etypes.h"

/* Memory table is 32 bits long. */
#define MEM_BLOCK_SZ (MEM_HEAP_SIZE / 32)
static uint32_t g_mem_table = 0;
static uint8_t g_heap[MEM_HEAP_SIZE] = { 0 };

void* malloc( size_t sz ) {
   uint8_t bit_cmp = 1;
   uint8_t page = 0;
   uint8_t* mem_table = (uint8_t*)&g_mem_table;
   uint16_t addr = 0;
   
   do {
      if( !(bit_cmp & *mem_table) ) {
         /* See if there are enough free blocks to fill the request. */

         return &(g_heap[addr]);
      }
      addr++;
   } while( page < 4 ); /* 4 * 8 = 32 bits */

   return NULL;
}

void free( void* ptr ) {

}

void* memset( void* s, int c, size_t n ) {
   return NULL;
}

char* strtok( char* str, size_t sz, const char* delim ) {
   size_t i, j, m, idx = 0;

   m = strlen( delim );

   /* Iterate tokenized string. */
   for( i = 0 ; sz > i ; i++ ) {
      /* Iterate delims. */
      for( j = 0 ; m > j ; j++ ) {
         if( '\0' == str[i] ) {
            idx = i + 1;
         } if( str[i] == delim[j] ) {
            /* Set a NULL and return the address of this token. */
            str[i] = '\0';
            return &(str[idx]);
         }
      }
   }

   return NULL;
}

union mvalue {
   UTOA_T d;
   char c;
   uint8_t x;
   void* p;
   char* s;
};

static void pad( char pad, size_t len, FILE* f ) {
   uint8_t i = 0;

   if( 0 >= len || '\0' == pad ) {
      return;
   }

   while( 0 == len || i < len) {
      putc( pad, f );
      i++;
   }
}

void fprintf( FILE* f, const char* pattern, ... ) {
   va_list args;
   int i = 0, j = 0;
   char last = '\0';
   union mvalue* spec = NULL;
   char* buffer = NULL;
   size_t pad_len = 0;
   char c;
   uint8_t pad_char = ' ';

   va_start( args, pattern );

   for( i = 0 ; '\0' != pattern[i] ; i++ ) {
      c = pattern[i]; /* Separate so we can play tricks below. */
 
      if( '%' == last ) {
         /* Conversion specifier encountered. */
         spec = malloc( sizeof( union mvalue ) );
         buffer = malloc( UTOA_DIGITS_MAX + 1 );
         switch( pattern[i] ) {
            case 's':
               spec->s = va_arg( args, char* );

               /* Print padding. */
               pad_len -= strlen( spec->s );
               pad( pad_char, pad_len, f );

               /* Print string. */
               j = 0;
               while( '\0' != spec->s[j] ) {
                  putc( spec->s[j++], f );
               }
               break;

            case 'd':
               spec->d = va_arg( args, UTOA_T );
               
               /* Print padding. */
               pad_len -= udigits( spec->d, 10 );
               pad( pad_char, pad_len, f );

               /* Print number. */
               if( NULL != utoa( spec->d, buffer, 10 ) ) {
                  j = 0;
                  while( '\0' != buffer[j] && j <= UTOA_DIGITS_MAX ) {
                     putc( buffer[j], f );
                     j++;
                  }
               }
               break;

            case 'x':
               spec->d = va_arg( args, int );

               /* Print padding. */
               pad_len -= udigits( spec->d, 16 );
               pad( pad_char, pad_len, f );

               /* Print number. */
               if( NULL != utoa( spec->d, buffer, 16 )
               ) {
                  putc( '0', f );
                  putc( 'x', f );
                  j = 0;
                  while( '\0' != buffer[j] && j <= UTOA_DIGITS_MAX ) {
                     putc( buffer[j], f );
                     j++;
                  }
               }
               break;

            case 'c':
               spec->c = va_arg( args, int );

               /* Print padding. */
               pad( pad_char, pad_len, f );

               /* Print char. */
               putc( spec->c, f );
               break;

#ifndef CONSOLE_NO_PRINTF_PTR
            case 'p':
               spec->p = va_arg( args, void* );

               /* Print padding. */
               pad_len -= udigits( (uintptr_t)spec->p, 16 );
               pad( pad_char, pad_len, f );

               /* Print pointer as number. */
               if( NULL != utoa( (uintptr_t)spec->p, buffer, 16 ) ) {
                  j = 0;
                  while( '\0' != buffer[j] && j <= UTOA_DIGITS_MAX ) {
                     putc( buffer[j], f );
                     j++;
                  }
               }
               break;
#endif /* !CONSOLE_NO_PRINTF_PTR */

            case '%':
               last = '\0';
               putc( '%', f );
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
         free( spec );
         free( buffer );
      } else if( '%' != c ) {
         pad_char = ' '; /* Reset padding. */
         pad_len = 0; /* Reset padding. */
         /* Print non-escape characters verbatim. */
         switch( c ) {
         case '\n':
#if CONSOLE_NEWLINE_R_N == CONSOLE_NEWLINE
            putc( '\r', f );
#endif /* CONSOLE_NEWLINE */
            putc( '\n', f );
            break;

         default:
            putc( c, f );
            break;
         }
      }

      last = c;
   }
}

