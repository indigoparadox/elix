
#ifndef ALPHA_H
#define ALPHA_H

#include <stdint.h>
#include <stddef.h>

#include "mem.h"

#define UINT8_DIGITS_MAX 8
#define INT_DIGITS_MAX 10

typedef MEMLEN_T STRLEN_T;

struct astring {
   STRLEN_T sz;
   STRLEN_T len;
   char data[];
} __attribute__( (packed) );

#define ASTR_NOT_FOUND -1

#define astring_l( str ) { sizeof( str ), sizeof( str ), str }
#define astring_append( str, c ) \
   if( str->len + 1 < str->sz ) { \
      str->data[str->len] = c; \
      str->len++; \
   }
#define astring_clear( str ) \
   mzero( str->data, str->sz ); \
   str->len = 0;
#define astring_sizeof( str ) \
   (sizeof( struct astring ) + sizeof( str ))

#define alpha_isprintable( c ) (' ' <= c && '~' >= c)
#define alpha_isdigit( c ) ('0' <= (c) && '9' >= (c))
#define alpha_islower( c ) ('a' <= (c) && 'z' >= (c))
#define alpha_isupper( c ) ('A' <= (c) && 'Z' >= (c))
#define alpha_isalnum( c ) \
	(alpha_isdigit( c ) || \
	alpha_islower( c ) || \
	alpha_isupper( c ))

#define stolower( c ) \
	(alpha_isupper( c ) ? c + ('A' - 'a') : c)

#define alpha_strlen( string, len ) alpha_charinstr( '\0', string, len )

#ifdef ALPHA_C
#define astring_const( id, str ) const struct astring id = astring_l( str )
#else
#define astring_const( id, str ) extern struct astring id
#endif /* STRINGS_C */

uint16_t alpha_atou( const struct astring* src, uint8_t base );
const char* alpha_tok( const struct astring* src, char sep, uint8_t idx );
STRLEN_T alpha_udigits( uint16_t num, uint8_t base );
int16_t alpha_utoa(
   uint16_t num, struct astring* str, STRLEN_T idx,
   STRLEN_T zero_pad_spaces, uint8_t base );
int16_t alpha_charinstr( char c, const struct astring* string );
struct astring* alpha_astring( uint8_t pid, MEM_ID mid, STRLEN_T len );
struct astring* alpha_astring_list_next( const struct astring* );
STRLEN_T alpha_cmp(
   const struct astring* str1, const struct astring* str2, char sep
);
STRLEN_T alpha_cmp_c(
   const char* cstr, STRLEN_T clen, const struct astring* astr, char sep
);
int8_t alpha_cmp_l(
   const struct astring* str, const struct astring list[], uint8_t len,
   char sep
);
int8_t alpha_cmp_cl(
   const char* cstr, STRLEN_T strlen, const struct astring list[], uint8_t len,
   char sep
);
/* void alpha_insertstr(
	char* dest, STRLEN_T dest_len, const char* src, int8_t* cursor
); */

#endif /* ALPHA_H */

