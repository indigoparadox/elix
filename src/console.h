
#ifndef CONSOLE_H
#define CONSOLE_H

#include <bstrlib/bstrlib.h>

void tprintf( const char* pattern, ... );
void tputs( const bstring str );

#endif /* CONSOLE_H */

