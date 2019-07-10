
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdarg.h>

#include "adhd.h"
#include "alpha.h"
#include "platform.h"

#if 0
#ifdef QD_CONSOLE_IN
#define twaitc() PPCONCAT( QD_CONSOLE_IN, _hit( g_console_dev_index ) )
#define tgetc() PPCONCAT( QD_CONSOLE_IN, _getc( g_console_dev_index ) )
#else
#define twaitc() keyboard_hit( g_console_dev_index )
#define tgetc() keyboard_getc( g_console_dev_index )
#endif /* QD_CONSOLE_IN */

#ifdef QD_CONSOLE_OUT
#define tputc( c ) PPCONCAT( QD_CONSOLE_OUT, _putc( g_console_dev_index, c ) )
#else
#define tputc( c ) display_putc( g_console_dev_index, c )
#endif /* QD_CONSOLE_OUT */
#endif

#ifndef CONSOLE_NEWLINE
#define CONSOLE_NEWLINE "\n"
#endif /* CONSOLE_NEWLINE */

#define console_const( id, str ) astring_const( id, str )

#define tputs( str ) tprintf( "%a", str )

void tputc( char c );
char tgetc();
char twaitc();
void tprintf( const char* pattern, ... );

#ifdef CONSOLE_C
uint8_t g_console_dev_index = 0;
#else
extern uint8_t g_console_dev_index;
#endif /* CONSOLE_C */

#endif /* CONSOLE_H */

