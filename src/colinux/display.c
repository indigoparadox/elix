
#include "../display.h"

#include <stdio.h>
#include <ncurses.h>

void display_set_colors( uint8_t fg, uint8_t bg ) {
}

void display_init() {
   /*initscr();
   cbreak();
   noecho();*/
}

void display_puts( const char* str ) {
   //printw( str );
   printf( "%s\n", str );
}

void display_putc_at( char c, int x, int y ) {
}

void display_putc( char c ) {
}

