
#include "cstd.h"

#include "etypes.h"

#ifndef MEM_HEAP_SIZE
#warning "Setting heap size to 320."
#define MEM_HEAP_SIZE 320
#endif /* MEM_HEAP_SIZE  */

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

size_t strlen( const char* str ) {
   size_t i = 0;
   while( '\0' != str[i] ) {
      i++;
   }
   return i;
}

int strncmp( const char* str1, const char* str2, size_t sz ) {
   size_t i = 0;

#ifdef CSTD_DEBUG_VERBOSE
   printf( "strncmp: \"%s\" vs \"%s\" (%d)\n", str1, str2, sz );
   fflush( 0 );
#endif /* CSTD_DEBUG_VERBOSE */

   for( i = 0 ; i < sz ; i++ ) {
      if( str1[i] < str2[i] ) {
         return str1[i] - str2[i];
      } else if( str1[i] > str2[i] ) {
         return str1[i] - str2[i];
      } else if( str1[i] == '\0' && str2[i] == '\0' ) {
         return 0;
      }
   }


   return 0;
}

char* strtok_r( char* str, const char* delim, char** saveptr ) {
   char* start = (str != NULL ? str : *saveptr);
   char* iter = start;
   int i = 0;

   /* str and *saveptr are both NULL. Nothing to iterate! */
   if( NULL == start ) {
      return NULL;
   }

   /* Hunt for the delim chars. */
   while( '\0' != *iter ) {
      for( i = 0 ; strlen( delim ) > i ; i++ ) {
         if( *iter == delim[i] ) {
            /* Delim found. */
            *iter = '\0';
            *saveptr = ++iter;
            return start;
         }
      }
      iter++;
   }

   /* Reached the end of str. */
   *saveptr = NULL;

   return start;
}

void strnreplace( char* str, size_t sz, const char* src, const char* r ) {
   int i = 0, r_iter = 0, sr_diff = 0;

   for( i = 0 ; sz > i ; i++ ) {

      if( NULL != src && sz - i < strlen( src ) ) {
         /* Not enough chars left to compare, so fail automatically. */
         break;

      } else if(
         (NULL == src && '\0' == str[i]) ||
         /* Max out at source length. */
         (NULL != src && 0 == strncmp( &(str[i]), src, strlen( src ) ))
      ) {
#ifdef CSTD_DEBUG_VERBOSE
         printf( "found\n" );
#endif /* CSTD_DEBUG_VERBOSE */
         if( NULL != src ) {
            sr_diff = strlen( r ) - strlen( src );
         } else {
            sr_diff = strlen( r ) - 1; /* 1 NULL */
         }
         /* Move chars at the end of the string forward to make room. */
         for( r_iter = sz ; r_iter > i + sr_diff ; r_iter-- ) {
#ifdef CSTD_DEBUG_VERBOSE
            printf( "move %c to %c\n",
               str[r_iter - sr_diff] ? str[r_iter - sr_diff] : 'X',
               str[r_iter] ? str[r_iter] : 'X' );
            fflush( 0 );
#endif /* CSTD_DEBUG_VERBOSE */
            str[r_iter] = str[r_iter - sr_diff];
         }

         /* Replace old text/blanks with new string contents. */
         for( r_iter = 0 ; strlen( r ) > r_iter ; r_iter++ ) {
#ifdef CSTD_DEBUG_VERBOSE
            printf( "replace %c with %c\n", str[i + r_iter], r[r_iter] );
            fflush( 0 );
#endif /* CSTD_DEBUG_VERBOSE */
            str[i + r_iter] = r[r_iter];
         }
         i += strlen( r ) - 1;
      }
   }
}

void strmtrunc( char* str, size_t start, size_t diff ) {
   int i = 0;

   while( diff > 0 ) {
      for( i = start ; strlen( str ) > i ; i++ ) {
         /* Grab the null past the end of strlen, too. */
         str[i] = str[i + 1];
      }
      diff--;
   }
}

unsigned int atou( const char* str, int base ) {
   unsigned int value = 0;
   int i = 0, len;

   len = strlen( str );

   while( isalnum( *str ) && i < len ) {
      value *= base;

      if( '9' >= *str ) {
         value += (*str - '0');
      } else if( 'z' >= *str ) {
         value += (*str - 'a');
      } else if( 'Z' >= *str ) {
         value += (*str - 'A');
      }
      i++;
      str++;
   }

   return value;
}

char* utoan( unsigned int num, char* dest, size_t dest_sz, int base ) {
   uint8_t rem;
   size_t digits;
   size_t digits_done = 0;
   size_t dest_idx = 0;

   digits = udigits( num, base );
   assert( (0 == num && 1 == digits) || (0 < num && 0 < digits) );
   assert( digits < dest_sz );

   /* Handle 0 explicitly, otherwise empty string is printed for 0. */
   if( 0 == num ) {
      dest[0] = '0';
      digits_done++;
   }

   dest_idx += digits;
   while( 0 != num ) {
      /* Get the 1's place. */
      rem = num % base;
      dest[--dest_idx] = (9 < rem) ?
         /* > 10, so it's a letter. */
         (rem - 10) + 'a' :
         /* < 10, so it's a number. */
         rem + '0';
      /* Move the next place value into range. */
      num /= base;
      digits_done++;
   }
   while( digits_done < digits ) {
      dest[--dest_idx] = '0';
      digits_done++;
   }
   dest[digits] = '\0';

   return dest;
}

size_t udigits( unsigned int num, int base ) {
   STRLEN_T digits = 0;
   while( 0 < num ) {
      num /= base;
      digits++;
   }
   if( 0 == digits ) {
      digits = 1; /* 0 */
   }
   return digits;
}

union mvalue {
   UTOA_T d;
   char c;
   uint8_t x;
   void* p;
   char* s;
};

static void pad( char pad, int len, int f ) {
   uint8_t i = 0;

   if( 0 >= len || '\0' == pad ) {
      return;
   }

   while( 0 == len || i < len) {
      putc( pad, f );
      i++;
   }
}

void fprintf( int f, const char* pattern, ... ) {
   va_list args;
   int i = 0, j = 0;
   char last = '\0';
   union mvalue spec;
   char buffer[UTOA_DIGITS_MAX + 1];
   int pad_len = 0; /* Needs to be able to go negative. */
   char c;
   uint8_t pad_char = ' ';

   va_start( args, pattern );

   for( i = 0 ; '\0' != pattern[i] ; i++ ) {
      c = pattern[i]; /* Separate so we can play tricks below. */

      if( '%' == last ) {
         /* Conversion specifier encountered. */
         switch( pattern[i] ) {
            case 's':
               spec.s = va_arg( args, char* );

               /* Print padding. */
               pad_len -= strlen( spec.s );
               pad( pad_char, pad_len, f );

               /* Print string. */
               j = 0;
               while( '\0' != spec.s[j] ) {
                  putc( spec.s[j++], f );
               }
               break;

            case 'd':
               spec.d = va_arg( args, UTOA_T );

               /* Print padding. */
               pad_len -= udigits( spec.d, 10 );
               pad( pad_char, pad_len, f );

               /* Print number. */
               if( NULL != utoan( spec.d, buffer, UTOA_DIGITS_MAX, 10 ) ) {
                  j = 0;
                  while( '\0' != buffer[j] && j <= UTOA_DIGITS_MAX ) {
                     putc( buffer[j], f );
                     j++;
                  }
               }
               break;

            case 'x':
               spec.d = va_arg( args, int );

               /* Print padding. */
               pad_len -= udigits( spec.d, 16 );
               pad( pad_char, pad_len, f );

               /* Print number. */
               if( NULL != utoan( spec.d, buffer, UTOA_DIGITS_MAX, 16 )
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
               spec.c = va_arg( args, int );

               /* Print padding. */
               pad( pad_char, pad_len, f );

               /* Print char. */
               putc( spec.c, f );
               break;

#ifndef CONSOLE_NO_PRINTF_PTR
            case 'p':
               spec.p = va_arg( args, void* );

               /* Print padding. */
               pad_len -= udigits( (uintptr_t)spec.p, 16 );
               pad( pad_char, pad_len, f );

               /* Print pointer as number. */
               if( NULL != utoan( (uintptr_t)spec.p, buffer, UTOA_DIGITS_MAX, 16 ) ) {
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

