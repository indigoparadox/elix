
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdarg.h>

void tprintf( const char* pattern, ... );
void tputsl( const char* str, uint8_t fg, uint8_t bg );
void tputs( const char* str );
int trepl_task( int pid );

#endif /* CONSOLE_H */

