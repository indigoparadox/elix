
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdarg.h>

void tprintf( const char* pattern, int len, ... );
void tputs( const char* str, int len );

#endif /* CONSOLE_H */

