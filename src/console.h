
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdarg.h>

#include "adhd.h"

#define REPL_LINE_SIZE_MAX 20

void tprintf( const char* pattern, ... );
void tputsl( const char* str, uint8_t fg, uint8_t bg );
void tputs( const char* str );
TASK_RETVAL trepl_task( TASK_PID pid );

#endif /* CONSOLE_H */

