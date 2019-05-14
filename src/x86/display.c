
#define DISPLAY_C
#include "../display.h"

static size_t display_cursor_row = 0;
static size_t display_cursor_col = 0;
static uint8_t display_color = 0;
static uint16_t* display_buffer = NULL;

/* Generic interface stuff. */

void display_set_colors( uint8_t fg, uint8_t bg ) {
   display_color = fg;
}

void display_init() {
   size_t x, y, index;
   display_set_colors( COLOR_GREEN, COLOR_BLUE );
   display_buffer = (uint16_t*)0xb8000;
   for( y = 0 ; DISPLAY_HEIGHT > y ; y++ ) {
      for( x = 0 ; DISPLAY_WIDTH > x ; x++ ) {
         index = y * DISPLAY_WIDTH + x;
         display_buffer[index] = ' ' | (display_color << 8);
      }
   }
}

void display_putc_at( char c, size_t x, size_t y ) {
   size_t index = 0;
   index = y * DISPLAY_WIDTH + x;
   display_buffer[index] = c | (display_color << 8);
}

void display_putc( char c ) {
   display_putc_at( c, display_cursor_col, display_cursor_row );
   if( DISPLAY_WIDTH == ++display_cursor_col ) {
      /* Overflow to the left and move to next row. */
      display_cursor_col = 0;
      if( DISPLAY_HEIGHT == ++display_cursor_row ) {
         display_cursor_row = 0;
      }
   }
}

void display_puts( const bstring str ) {
   size_t len = 0, i = 0;
   len = tstrlen( str );
   for( i = 0 ; len > i ; i++ ) {
      display_putc( str[i] );
   }
}

