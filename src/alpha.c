
#include "code16.h"

#define ALPHA_C
#include "alpha.h"
#include "console.h"

uint16_t alpha_atou_c( const char* ch, int len, uint8_t base ) {
   uint16_t value = 0;
   uint8_t i = 0;

   while( alpha_isalnum( *ch ) && i < len ) {
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

uint16_t alpha_atou( const struct astring* src, uint8_t base ) {
   return alpha_atou_c( src->data, src->len, base );
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

STRLEN_T alpha_utoa( UTOA_T num, struct astring* dest, uint8_t base ) {
   STRLEN_T len_out = 0;
   assert( NULL != dest );
   assert( 0 < dest->mem.sz );
   len_out = alpha_utoa_c(
      num, dest->data, dest->mem.sz - sizeof( struct astring ), base );
   assert( (0 == num && 0 == len_out) || (0 < num && 0 < len_out) );
   dest->len = len_out;
   return len_out;
}

STRLEN_T alpha_utoa_c(
   UTOA_T num, char* dest, STRLEN_T dest_sz, uint8_t base
) {
   uint8_t rem;
   STRLEN_T digits;
   STRLEN_T digits_done = 0;
   STRLEN_T dest_idx = 0;

   digits = alpha_udigits( num, base );
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

   return digits;
}

void alpha_replace( char c, char r, struct astring* string ) {
   STRLEN_T idx = 0;

   idx = alpha_charinstr_c( c, string->data, string->len );
   while( ASTR_NOT_FOUND != idx ) {
      string->data[idx] = r;
      idx = alpha_charinstr_c( c, string->data, string->len );
   }
}

STRLEN_T alpha_charinstr( char c, const struct astring* string ) {
   return alpha_charinstr_c( c, string->data, string->len );
}

STRLEN_T alpha_charinstr_c( char c, const char* string, STRLEN_T len ) {
   uint8_t i = 0;
   while( len > i ) {
      if( c == string[i] ) {
         return i;
      }
      i++;
   }
   /* Didn't find it! */
   return ASTR_NOT_FOUND;
}

void alpha_astring_clear( struct astring* str ) {
   assert( NULL != str );
   str->len = 0;
   mzero( str->data, str->mem.sz - sizeof( struct astring ) );
}

void alpha_astring_append( struct astring* str, char c ) {
   assert( NULL != str );
   assert( str->mem.sz > sizeof( struct astring ) );
   assert( str->len + sizeof( struct astring ) + 1 < str->mem.sz );

   str->data[str->len++] = c;

   /* Add a terminating NULL. */
   str->data[str->len] = '\0';
}

void alpha_astring_trunc( struct astring* str, STRLEN_T diff ) {
   assert( NULL != str );
   if( str->len - diff >= 0 ) {
      str->len -= diff;
      str->data[str->len] = '\0';
   }
}

void alpha_astring_rtrunc( struct astring* str, STRLEN_T diff ) {
   STRLEN_T i = 0;
   STRLEN_T new_len = str->len - diff;
   assert( NULL != str );
   if( str->len - diff >= 0 ) {
      for( i = 0 ; new_len > i ; i++ ) {
         str->data[i] = str->data[i + diff];
      }
      str->data[new_len] = '\0';
   }
}

struct astring*
alpha_astring( uint8_t pid, MEM_ID mid, STRLEN_T len, const char* str ) {
   struct astring* str_out = NULL;

   assert( NULL == str || len > sizeof( str ) );
   
   /* The mem struct is embedded at the front of the astring struct. */
   str_out = (struct astring*)mget_meta(
      pid, mid, sizeof( struct astring ) + len );
   assert( NULL != str_out );
   if( NULL != str && 0 < len ) {
      mcopy( str_out->data, str, len );
   }

   return str_out;
}

struct astring* alpha_astring_list_next( const struct astring* str_in ) {
   uint8_t* str_out = (uint8_t*)str_in;

   str_out += sizeof( struct astring );
   str_out += str_in->mem.sz;

   return (struct astring*)str_out;
}

STRLEN_T alpha_cmp(
   const struct astring* str1, const struct astring* str2, char sep,
   bool case_match, uint8_t len_match
) {
   return alpha_cmp_cc( str1->data, str1->len, str2->data, str2->len,
      sep, case_match, len_match );
}

STRLEN_T alpha_cmp_c(
   const char* cstr, STRLEN_T clen, const struct astring* astr, char sep,
   bool case_match, uint8_t len_match
) {
   return alpha_cmp_cc( cstr, clen, astr->data, astr->len, sep, case_match,
      len_match );
}

STRLEN_T alpha_cmp_cc(
   const char* cstr1, STRLEN_T clen1, const char* cstr2, STRLEN_T clen2, 
   char sep, bool case_match, uint8_t len_match
) {
   STRLEN_T i = 0;

#ifdef ALPHA_CMP_DEBUG
         tprintf( "\n(%s)[%d] vs (%s)[%d] [%c]\n",
            cstr1, clen1, cstr2, clen2, sep );
#endif /* ALPHA_CMP_DEBUG */

   while( /*
      cstr1[i] != sep && 
      (0 == clen1 || clen1 > i) &&
      cstr2[i] != sep && 
      (0 == clen2 || clen2 > i) &&
      '\0' != cstr1[i] &&
      '\0' != cstr2[i] */
      i < clen1 || i < clen2
   ) {
      if(
         i < len_match && (
            (cstr1[i] == sep && (
               cstr2[i] != sep &&
               cstr2[i] != '\0'
            )) ||
            (cstr1[i] == '\0' && (
               cstr2[i] != sep &&
               cstr2[i] != '\0'
            ))
         )
      ) {
#ifdef ALPHA_CMP_DEBUG
         tprintf( "(%d) len mismatch (0x%x vs 0x%x)\n", i, cstr1[i], cstr2[i] );
#endif /* ALPHA_CMP_DEBUG */
         return 1;
      } else if(
         i >= len_match && (
            (cstr1[i] == sep && cstr2[i] == sep) ||
            (cstr1[i] == sep && cstr2[i] == '\0') ||
            (cstr1[i] == '\0' && cstr2[i] == sep) ||
            (cstr1[i] == '\0' && cstr2[i] == '\0')
         )
      ) {
#ifdef ALPHA_CMP_DEBUG
         tprintf( "(%d) len fuzzmatch\n", i );
#endif /* ALPHA_CMP_DEBUG */
         //break;
         return 0;
      }

      if( case_match && cstr1[i] != cstr2[i] ) {
#ifdef ALPHA_CMP_DEBUG
         tprintf( "(%d) case mismatch (%c / %c)\n", i,
            cstr1[i], cstr2[i] );
#endif /* ALPHA_CMP_DEBUG */
         return 1;
      } else if(
         !case_match &&
         alpha_tolower( cstr1[i] ) != alpha_tolower( cstr2[i] )
      ) {
#ifdef ALPHA_CMP_DEBUG
         tprintf( "(%d) caseless mismatch ('%c' -> '%c' / '%c' -> '%c')\n", i,
            cstr1[i], alpha_tolower( cstr1[i] ),
            cstr2[i], alpha_tolower( cstr2[i] ) );
#endif /* ALPHA_CMP_DEBUG */
         return 1;
      }

      i++;
   }

#ifdef ALPHA_CMP_DEBUG
   tprintf( "de facto match\n" );
#endif /* ALPHA_CMP_DEBUG */
   return 0;
}

int8_t alpha_cmp_l(
   const struct astring* str, const struct astring list[], uint8_t len,
   char sep, bool case_match, uint8_t len_match
) {
   uint8_t idx = 0;

   for( idx = 0 ; len > idx ; idx++ ) {
      if( 0 == alpha_cmp( &(list[idx]), str, sep, case_match, len_match ) ) {
         return idx;
      }
   }

   return ASTR_NOT_FOUND;
}

int8_t alpha_cmp_cl(
   const char* cstr, STRLEN_T clen, const struct astring list[], uint8_t len,
   char sep, bool case_match, uint8_t len_match
) {
   uint8_t idx = 0;

   for( idx = 0 ; len > idx ; idx++ ) {
      if( 0 == alpha_cmp_c( cstr, clen, &(list[idx]), sep, case_match, len_match ) ) {
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

#if 0
uint16_t alpha_divide_evenly( uint16_t dividend, uint16_t divisor ) {
   if( 0 == divisor ) {
      return 0; /* TODO: Crash or something? */
   }
   while( 0 != divisor % divisor ) {
      divisor--;
   }
   return dividend / divisor;
}
#endif

