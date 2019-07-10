
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdarg.h>

#include "adhd.h"
#include "alpha.h"
#include "platform.h"

#ifndef QD_CONSOLE_IN_IDX
#define QD_CONSOLE_IN_IDX 0
#endif /* QD_CONSOLE_IN_IDX */

#ifndef QD_CONSOLE_OUT_IDX
#define QD_CONSOLE_OUT_IDX 0
#endif /* QD_CONSOLE_OUT_IDX */

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
uint8_t g_console_in_dev_index = QD_CONSOLE_IN_IDX;
uint8_t g_console_out_dev_index = QD_CONSOLE_OUT_IDX;
#else
extern uint8_t g_console_in_dev_index;
extern uint8_t g_console_out_dev_index;
#endif /* CONSOLE_C */

#endif /* CONSOLE_H */

