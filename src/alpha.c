
#define ALPHA_C
#include "alpha.h"
#include "console.h"
#include "strings.h"

uint16_t alpha_atou( const struct astring* src, uint8_t base ) {
   uint16_t value = 0;
   const char* ch = src->data;
   uint8_t i = 0;

   while( alpha_isalnum( *ch ) && i < src->len ) {
      value *= base;

      if( '9' >= *ch ) {
         value += (*ch - '0');
      } else if( 'z' >= *ch ) {
         value += (*ch - 'a');
      } else if( 'Z' >= *ch ) {
         value += (*ch - 'A');
      }
      i++;
      ch++;
   }

   return value;
}

const char* alpha_tok( const struct astring* src, char sep, uint8_t idx ) {
   uint8_t char_iter = 0;
   uint8_t tok_iter = 0;

   while( tok_iter < idx ) {
      while( src->data[char_iter] != sep ) {
         char_iter++;
         if( char_iter >= src->len ) {
            return NULL;
         }
      }
      char_iter++;
      tok_iter++;
   }

   return &(src->data[char_iter]);
}

/* Return the number of digits in a number. */
STRLEN_T alpha_udigits( uint16_t num, uint8_t base ) {
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

int16_t alpha_utoa(
   uint16_t num, struct astring* dest, STRLEN_T dest_idx,
   STRLEN_T zero_pad_spaces, uint8_t base
) {
   uint8_t rem;
   STRLEN_T digits;
   STRLEN_T digits_done = 0;
   int16_t places_diff = 0;

   digits = alpha_udigits( num, base );
   if( digits < zero_pad_spaces ) {
      digits = zero_pad_spaces;
   }
   if( digits >= (dest_idx + dest->sz) ) {
      /* Not enough space in dest to hold the number. */
      return -1;
   }

   /* If the string length is less than number of digits, bump it up. */
   places_diff = digits - (dest_idx + dest->len);
   if( 0 < places_diff ) {
      dest->len += places_diff;
   }

   /* Handle 0 explicitly, otherwise empty string is printed for 0. */
   if( 0 == num ) {
      dest->data[dest_idx] = '0';
   }

   dest_idx += digits;
   while( 0 != num ) {
      /* Get the 1's place. */
      rem = num % base;
      dest->data[--dest_idx] = (9 < rem) ? 
         /* > 10, so it's a letter. */
         (rem - 10) + 'a' :
         /* < 10, so it's a number. */
         rem + '0';
      /* Move the next place value into range. */
      num /= base;
      digits_done++;
   }
   while( digits_done < digits ) {
      dest->data[--dest_idx] = '0';
      digits_done++;
   }

   return digits;
}

int16_t alpha_charinstr( char c, const struct astring* string ) {
   uint8_t i = 0;
   while( string->len > i ) {
      if( c == string->data[i] ) {
         return i;
      }
      i++;
   }
   /* Didn't find it! */
   return -1;
}

struct astring* alpha_astring( uint8_t pid, MEM_ID mid, STRLEN_T len ) {
   struct astring* str_out = NULL;
   
   str_out = mget( pid, mid, sizeof( struct astring ) + len );
   if( 0 == str_out->sz ) {
      str_out->sz = len;
   }

   return str_out;
}

STRLEN_T alpha_cmp_c( const char* cstr, const struct astring* astr, char sep ) {
   STRLEN_T i = 0;
   while( cstr[i] != sep && '\0' != cstr[i] && i < astr->len ) {
      if( astr->data[i] != cstr[i] ) {
         return 1;
      }
      i++;
   }
   return 0;
}

#if 0
STRLEN_T alpha_insertstr(
   struct astring* dest, const struct astring* src, int8_t* cursor
) {
   STRLEN_T str_len;
   STRLEN_T insert_len;
   int8_t move_cur;
   STRLEN_T insert_offset;

   str_len = alpha_strlen( dest );
   insert_len = alpha_strlen( dest ); /* TODO: Is this correct? */
   str_len += dest->len - 2; /* -1 for the token we're replacing,
                              * and -1 for the \0. */
   if( dest->sz < src->len + *cursor ) {
      return 0;
   }
   insert_offset = *cursor + insert_len;

   /* TODO: Consolidate this with mshift to save space. */
   for(
      move_cur = str_len ;
      move_cur >= insert_offset ; /* Overwrite the token. */
      move_cur--
   ) {
      dest[move_cur] = dest[move_cur - insert_len + 1];
   }

   /* Perform the insertion into the now vacated space. */
   alpha_copystr( &(dest[*cursor]), insert_len, src );

   return str_len;
}
#endif

