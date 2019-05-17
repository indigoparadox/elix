
#include "alpha.h"

void alpha_tolower( char* str, uint8_t length ) {
   while( length-- && '\0' != *str ) {
      if( alpha_isupper( (*str) ) ) {
         *str -= 32; /* Distance between ASCII upper/lower. */
      }
      str++;
   }
}

uint16_t alpha_atou( const char* string, uint8_t len, uint8_t base ) {
   uint16_t value = 0;
   char ch = *string;
	uint8_t i = 0;

	while( alpha_isalnum( ch ) && i < len ) {
		value *= base;

      if( '9' >= ch ) {
         value += (ch - '0');
      } else if( 'z' >= ch ) {
         value += (ch - 'a');
      } else if( 'Z' >= ch ) {
         value += (ch - 'A');
      }
		i++;
      ch = *(++string);
   }

	return value;
}

uint8_t alpha_utoa( uint16_t num, char* str, uint8_t len, uint8_t base ) {
	uint8_t rem;
	uint8_t digits = 1;
	uint16_t radix_base = base;

   /* Figure out the number of digits in the number. */
	while( num >= radix_base ) {
      if( digits >= len ) {
         return 0;
      }
		radix_base *= radix_base;
		digits++;
	}

	/* Handle 0 explicitly, otherwise empty string is printed for 0. */
	if( 0 == num ) {
		str[0] = '0';
	}

   /* We don't need len anymore, so let's reuse it. */
	len = digits;

	while( 0 != num ) {
		/* Get the 1's place. */
		rem = num % base;
		str[--digits] = (9 < rem) ? 
			/* > 10, so it's a letter. */
			(rem - 10) + 'a' :
			/* < 10, so it's a number. */
			rem + '0';
		/* Move the next place value into range. */
		num /= base;
	}

	return len;
}

int8_t alpha_charinstr( char c, const char* string, uint8_t len ) {
	uint8_t i = 0;
	while( len > i ) {
   	if( c == string[i] ) {
         return i;
      }
		i++;
	}
   /* Didn't find it! */
	return -1;
}

BOOL alpha_insertstr(
	char* dest, STRLEN_T dest_len, const char* src, int8_t* cursor
) {
	STRLEN_T str_len;
	STRLEN_T insert_len;
	int8_t move_cur;
	STRLEN_T insert_offset;

	str_len = alpha_strlen( dest, dest_len );
	insert_len = alpha_strlen( src, dest_len ); /* TODO: Is this correct? */
	str_len += insert_len - 2; /* -1 for the token we're replacing, 	*/
										/* and -1 for the \0.						*/
	if( dest_len < str_len ) {
		return FALSE;
	}
	insert_offset = *cursor + insert_len;
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

