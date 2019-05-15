
#ifndef DEBUG_H
#define DEBUG_H

#include "display.h"
#include "console.h"

#define derror( msg ) tputsl( msg "\n", COLOR_RED, COLOR_BLACK )
/* TODO: Do this in red, too! */
#define deprintf( msg, ... ) tprintf( msg "\n", __VA_ARGS__ )
#define dinfo( msg ) tputs( msg )
#define diprintf( msg, ... ) tprintf( msg "\n", __VA_ARGS__ )
#ifdef DEBUG
#define ddebug( msg ) tputs( msg )
#define ddprintf( msg, ... ) tprintf( msg, __VA_ARGS__ )
#else
#define ddebug( msg ) 
#define ddprintf( msg, ... )
#endif /* DEBUG */

#endif /* DEBUG_H */

