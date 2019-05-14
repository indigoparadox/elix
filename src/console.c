
#include "console.h"
#include "display.h"

void tputs( const bstring str ) {
#ifndef HEADLESS
   display_puts( str );
#endif /* HEADLESS */
}

