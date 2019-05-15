
#include "display.h"
#include "console.h"

void kmain() {
#ifndef HEADLESS
   display_init();
#endif /* HEADLESS */
   tputs( "hello\n", 6 );
}

