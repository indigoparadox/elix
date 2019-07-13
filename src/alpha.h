
#ifndef ALPHA_H
#define ALPHA_H

/*! \file alpha.h */

#include <stdbool.h>

#include "mem.h"

#define PPCONCAT_I( a, b ) a##b
#define PPCONCAT( a, b ) PPCONCAT_I( a, b )

#define astring_l( str ) { { 0, 0, sizeof( str ), {} }, sizeof( str ), str }
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

#define alpha_tolower( c ) \
	(alpha_isupper( c ) ? c + ('a' - 'A') : c)

#define alpha_strlen_c( string, len ) alpha_charinstr_c( '\0', string, len )
#define alpha_wordlen( string ) alpha_charinstr( ' ', string )
#define alpha_wordlen_c( string, len ) alpha_charinstr_c( ' ', string, len )

#ifdef ALPHA_C
#define astring_const( id, str ) const struct astring id = astring_l( str )
#else
#define astring_const( id, str ) extern struct astring id
#endif /* STRINGS_C */

#define alpha_16_lobyte( i ) ((i) & 0xFF)
#define alpha_16_hibyte( i ) ((i) >> 8)

uint16_t alpha_atou_c( const char* src, int len, uint8_t base );
uint16_t alpha_atou( const struct astring* src, uint8_t base );
const char* alpha_tok( const struct astring* src, char sep, uint8_t idx );
STRLEN_T alpha_udigits( UTOA_T num, uint8_t base );
STRLEN_T alpha_utoa(
   UTOA_T num, struct astring* str, STRLEN_T idx,
   STRLEN_T zero_pad_spaces, uint8_t base );
STRLEN_T alpha_charinstr( char c, const struct astring* string );
STRLEN_T alpha_charinstr_c( char c, const char* string, STRLEN_T len );
void alpha_astring_append( struct astring* str, char c );
void alpha_astring_trunc( struct astring* str, STRLEN_T diff );
void alpha_astring_clear( struct astring* str );
struct astring* alpha_astring(
   uint8_t pid, MEM_ID mid, STRLEN_T len, const char* str );
struct astring* alpha_astring_list_next( const struct astring* );
STRLEN_T alpha_cmp(
   const struct astring* str1, const struct astring* str2, char sep,
   bool case_match, uint8_t len_match
);
STRLEN_T alpha_cmp_c(
   const char* cstr, STRLEN_T clen, const struct astring* astr, char sep,
   bool case_match, uint8_t len_match
);
STRLEN_T alpha_cmp_cc(
   const char* cstr1, STRLEN_T clen1, const char* cstr2, STRLEN_T clen2, 
   char sep, bool case_match, uint8_t len_match
);

/** \brief  Return the index of the current string in the given list, or
 *          ASTR_NOT_FOUND if string is not in list.
 */
int8_t alpha_cmp_l(
   const struct astring* str, const struct astring list[], uint8_t len,
   char sep, bool case_match, uint8_t len_match
);

/** \brief  Return the index of the current string in the given list, or
 *          ASTR_NOT_FOUND if string is not in list.
 */
int8_t alpha_cmp_cl(
   const char* cstr, STRLEN_T strlen, const struct astring list[], uint8_t len,
   char sep, bool case_match, uint8_t len_match
);
/* void alpha_insertstr(
	char* dest, STRLEN_T dest_len, const char* src, int8_t* cursor
); */

uint16_t alpha_divide_evenly( uint16_t dividend, uint16_t divisor );

#endif /* ALPHA_H */

