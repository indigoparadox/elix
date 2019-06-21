
#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef USE_CONSOLE

#include <stdint.h>
#include <stdarg.h>

#include "adhd.h"
#include "alpha.h"

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

#define tputc( c ) display_putc( c )
#define twaitc() keyboard_hit()
#define tgetc() keyboard_getc()

#define console_const( id, str ) astring_const( id, str )

int trepl_service( char* cli );
void tprintf( const char* pattern, ... );
void tputs( const struct astring* str );
TASK_RETVAL trepl_task();

#else /* USE_CONSOLE */

#define console_const( id, str )
#define tprintf( msg, ... )
#define tputs( str )

#endif /* USE_CONSOLE */

#endif /* CONSOLE_H */

