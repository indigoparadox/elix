
#include "../display.h"

#include <stdio.h>

void display_init() {

}

void display_puts( const bstring str ) {
   printf( "%s", bdata( str ) );
}

