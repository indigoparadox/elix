
#define ALPHA_C
#include "alpha.h"
#include "console.h"
#include "strings.h"

#ifdef DEBUG
#include <assert.h>
#else
#define assert( x )
#endif /* DEBUG */

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
STRLEN_T alpha_udigits( UTOA_T num, uint8_t base ) {
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

STRLEN_T alpha_utoa(
   UTOA_T num, struct astring* dest, STRLEN_T dest_idx,
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

void alpha_astring_clear( TASK_PID pid, MEM_ID mid ) {
   STRLEN_T zero = 0;
   meditprop(
      pid, mid, offsetof( struct astring, len ), sizeof( STRLEN_T ), &zero );
}

void alpha_astring_append( TASK_PID pid, MEM_ID mid, char c ) {
   const struct astring* str = NULL;
   STRLEN_T new_strlen = 0;

   str = mget( pid, mid, MGET_NO_CREATE );
   if( NULL == str ) {
      return;
   }

   if( str->len + 1 < str->sz ) {
      meditprop(
         pid, mid, offsetof( struct astring, data  ) + str->len,
         sizeof( char ), &c );

      /* Add a terminating NULL. */
      c = '\0';
      meditprop(
         pid, mid, offsetof( struct astring, data  ) + str->len + 1,
         sizeof( char ), &c );

      /* Bookkeeping. */
      new_strlen = str->len + 1;
      meditprop(
         pid, mid, offsetof( struct astring, len  ),
         sizeof( STRLEN_T ), &new_strlen );
   }
}

const struct astring* alpha_astring(
   uint8_t pid, MEM_ID mid, STRLEN_T len, char* str
) {
   const struct astring* str_out = NULL;

   assert( NULL == str || len > sizeof( str ) );
   
   //mset( pid, mid, sizeof( struct astring ) + len, str );
   str_out = mget( pid, mid, sizeof( struct astring ) + len );
   if( 0 == str_out->sz ) {
      meditprop(
         pid, mid, offsetof( struct astring, sz ), sizeof( STRLEN_T ), &len );
   }

   return str_out;
}

const struct astring* alpha_astring_list_next( const struct astring* str_in ) {
   uint8_t* str_out = (uint8_t*)str_in;

   str_out += sizeof( struct astring );
   str_out += str_in->sz;

   return (struct astring*)str_out;
}

STRLEN_T alpha_cmp(
   const struct astring* str1, const struct astring* str2, char sep
) {
   STRLEN_T i = 0;
   while(
      str1->data[i] != sep && str2->data[i] != sep &&
      i < str1->len && i < str2->len
   ) {
      if( str1->data[i] != str2->data[i] ) {
         return 1;
      }
      i++;
   }
   return 0;
}

STRLEN_T alpha_cmp_c(
   const char* cstr, STRLEN_T clen, const struct astring* astr, char sep
) {
   STRLEN_T i = 0;
   while(
      cstr[i] != sep && 
      clen > i &&
      astr->data[i] != sep && 
      astr->len > i &&
      '\0' != cstr[i]
   ) {
      if( astr->data[i] != cstr[i] ) {
         return 1;
      }
      i++;
   }
   return 0;
}

STRLEN_T alpha_cmp_cc(
   const char* cstr1, STRLEN_T clen1, const char* cstr2, STRLEN_T clen2, 
   char sep
) {
   STRLEN_T i = 0;
   while(
      cstr1[i] != sep && 
      (0 == clen1 || clen1 > i) &&
      cstr2[i] != sep && 
      (0 == clen2 || clen2 > i) &&
      '\0' != cstr1[i] &&
      '\0' != cstr2[i]
   ) {
      if( cstr1[i] != cstr2[i] ) {
         return 1;
      }
      i++;
   }
   return 0;
}

/** \brief  Return the index of the current string in the given list, or
 *          ASTR_NOT_FOUND if string is not in list.
 */
int8_t alpha_cmp_l(
   const struct astring* str, const struct astring list[], uint8_t len,
   char sep
) {
   uint8_t idx = 0;

   for( idx = 0 ; len > idx ; idx++ ) {
      if( 0 == alpha_cmp( &(list[idx]), str, sep ) ) {
         return idx;
      }
   }

   return ASTR_NOT_FOUND;
}

/** \brief  Return the index of the current string in the given list, or
 *          ASTR_NOT_FOUND if string is not in list.
 */
int8_t alpha_cmp_cl(
   const char* cstr, STRLEN_T clen, const struct astring list[], uint8_t len,
   char sep
) {
   uint8_t idx = 0;

   for( idx = 0 ; len > idx ; idx++ ) {
      if( 0 == alpha_cmp_c( cstr, clen, &(list[idx]), sep ) ) {
         return idx;
      }
   }

   return ASTR_NOT_FOUND;
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

uint16_t alpha_divide_evenly( uint16_t dividend, uint16_t divisor ) {
   if( 0 == divisor ) {
      return 0; /* TODO: Crash or something? */
   }
   while( 0 != divisor % divisor ) {
      divisor--;
   }
   return dividend / divisor;
}

