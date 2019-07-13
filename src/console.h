
#ifndef CONSOLE_H
#define CONSOLE_H

/*! \file alpha.h */

#include <stdarg.h>

#include "alpha.h"

#ifndef CONSOLE_NEWLINE
/*! The default newline string, if none is defined in the CFLAGS.
 *  Some platforms may require e.g. '\r' or '\r\n' or something else.
 */
#define CONSOLE_NEWLINE "\n"
#endif /* CONSOLE_NEWLINE */

#define console_const( id, str ) astring_const( id, str )

#define tputs( str ) tprintf( "%a", str )

void tputc( char c );
char tgetc();
char twaitc();
void tprintf( const char* pattern, ... );

#include "globals.h"

#endif /* CONSOLE_H */

