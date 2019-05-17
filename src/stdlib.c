
#include "stdlib.h"

#include <stdint.h>

uint16_t satou( const char* string, uint8_t len, uint8_t base ) {
   uint16_t value = 0;
   char ch = *string;
#ifndef BOUNDS_CHECK_DISABLED
	uint8_t i = 0;
#endif /* BOUNDS_CHECK_DISABLED */

#ifndef BOUNDS_CHECK_DISABLED
	while( mispos_is_alpha_digit( ch ) && i < len ) {
#else
	while( mispos_is_alpha_digit( ch ) ) {
#endif /* BOUNDS_CHECK_DISABLED */
		value *= base;

      if( '9' >= ch ) {
         value += (ch - '0');
      } else if( 'z' >= ch ) {
         value += (ch - 'a');
      } else if( 'Z' >= ch ) {
         value += (ch - 'A');
      }
#ifndef BOUNDS_CHECK_DISABLED
		i++;
#endif /* BOUNDS_CHECK_DISABLED */
      ch = *(++string);
   }

	return value;
}

uint8_t sutoa( uint16_t num, char* str, uint8_t len, uint8_t base ) {
	uint8_t rem;
	uint8_t digits = 1;
	uint16_t radix_base = base;

   /* Figure out the number of digits in the number. */
	while( num >= radix_base ) {
		radix_base *= radix_base;
		digits++;
	}

#ifndef BOUNDS_CHECK_DISABLED
	if( digits >= len ) {
		return 0;
	}
#endif /* BOUNDS_CHECK_DISABLED */

	/* Process individual digits. */
   if( system_status( STATUS_NO_TERM_TOA_FUNCS ) ) {
      str[digits] = ' ';
   } else {
      str[digits] = '\0'; /* Append string terminator. */
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

