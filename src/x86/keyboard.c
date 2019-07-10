
#include "../keyboard.h"
#include "../kernel.h"

__attribute__( (constructor( CTOR_PRIO_DISPLAY )) )
static void keyboard_init() {
}

void keyboard_shutdown() {
}

static int keyboard_hit( uint8_t dev_index ) {
   return 0;
}

char keyboard_getc( uint8_t dev_index, bool wait ) {
   if( wait ) {
      return keyboard_hit( dev_index );
   }
   return 0;
}


