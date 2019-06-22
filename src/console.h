
#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef USE_CONSOLE

#include <stdint.h>
#include <stdarg.h>

#include "adhd.h"
#include "alpha.h"
#include "uart.h"

#define REPL_LINE_SIZE_MAX 20

#define CONSOLE_OP_NOOP 0x0
#define CONSOLE_OP_PUSH 0x1
#define CONSOLE_OP_POP  0x2
#define CONSOLE_OP_ADD  0x3
#define CONSOLE_OP_SUB  0x4
#define CONSOLE_OP_MUL  0x5
#define CONSOLE_OP_DIV  0x6
#define CONSOLE_OP_MOD  0x7

#define CONSOLE_FLAG_INITIALIZED    0x01

#define VERSION "2019.21"

#include "keyboard.h"
#include "display.h"

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

#define console_const( id, str ) astring_const( id, str )

#define tputs( str ) tprintf( "%a", str )

void tprintf( const char* pattern, ... );
TASK_RETVAL trepl_task();

#else /* USE_CONSOLE */

#define console_const( id, str )
#define tprintf( msg, ... )
#define tputs( str )

#endif /* USE_CONSOLE */

#ifdef CONSOLE_C
uint8_t g_console_dev_index = 0;
#else
extern uint8_t g_console_dev_index;
#endif /* USE_CONSOLE */

#endif /* CONSOLE_H */

