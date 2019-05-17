
#ifndef ALPHA_H
#define ALPHA_H

#include <stdint.h>

typedef uint8_t STRLEN_T;

struct astring {
   STRLEN_T sz;
   STRLEN_T len;
   char data[];
};

#define astring_l( str ) { sizeof( str ), sizeof( str ), str }
#define astring_append( str, c ) \
   if( str->len + 1 < str->sz ) { \
      str->data[str->len] = c; \
      str->len++; \
   }

#define alpha_isprintable( c ) (' ' <= c && '~' >= c)
#define alpha_isdigit( c ) ('0' <= (c) && '9' >= (c))
#define alpha_islower( c ) ('a' <= (c) && 'z' >= (c))
#define alpha_isupper( c ) ('A' <= (c) && 'Z' >= (c))
#define alpha_isalnum( c ) \
	(alpha_isdigit( c ) || \
	alpha_islower( c ) || \
	alpha_isupper( c ))

#define alpha_clear( str ) \
   mzero( str->data, str->sz ); \
   str->len = 0;

#define stolower( c ) \
	(alpha_isupper( c ) ? c + ('A' - 'a') : c)

#define alpha_strlen( string, len ) alpha_charinstr( '\0', string, len )

uint16_t alpha_atou( const char* string, STRLEN_T len, uint8_t base );
STRLEN_T alpha_utoa( uint16_t num, char* str, STRLEN_T len, uint8_t base );
STRLEN_T alpha_charinstr( char c, const char* string, STRLEN_T len );
void alpha_insertstr(
	char* dest, STRLEN_T dest_len, const char* src, int8_t* cursor
);

#endif /* ALPHA_H */

