
#include "string.h"
#include "display.h"
#include "console.h"

struct tagbstring hello = bsStatic( "Hello!" );

void kmain() {
#ifndef HEADLESS
   display_init();
#endif /* HEADLESS */
   tputs( &hello );
}

