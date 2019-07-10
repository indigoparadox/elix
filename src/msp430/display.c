
#include "../display.h"
#include "../kernel.h"
#include "../io.h"

void display_set_colors( uint8_t fg, uint8_t bg ) {
}

__attribute__( (constructor( CTOR_PRIO_DISPLAY )) )
static void display_init() {

}

void display_putc_at( char c, int x, int y ) {
}

void display_putc( uint8_t dev_index, char c ) {
}

__attribute__( (destructor( CTOR_PRIO_DISPLAY )) )
static void display_shutdown() {

}

