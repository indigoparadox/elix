
#include "../code16.h"

#include "x86bios.h"

#include "../display.h"
#include "../kernel.h"
#include "../io.h"

static int display_cursor_row = 0;
static int display_cursor_col = 0;
static uint8_t display_color = 0;
static uint16_t* display_buffer = NULL;

/* Generic interface stuff. */

void display_set_colors( uint8_t fg, uint8_t bg ) {
   display_color = fg;
}

void display_init() {
   int x, y, index;
   display_set_colors( COLOR_WHITE, COLOR_BLACK );
   display_buffer = (uint16_t*)0xb800;
   for( y = 0 ; DISPLAY_HEIGHT > y ; y++ ) {
      for( x = 0 ; DISPLAY_WIDTH > x ; x++ ) {
         index = y * DISPLAY_WIDTH + x;
         display_buffer[index] = ' ' | (display_color << 8);
      }
   }
}

//__NOINLINE
//__REGPARM
void display_putc( char c ) {
   __asm__ __volatile__ ("int  $0x10" : : "a"(0x0E00 | c), "b"(7));
   if( '\n' == c ) {
      __asm__ __volatile__ ("int  $0x10" : : "a"(0x0E00 | '\r'), "b"(7));
   }
   //display_putc_at( c, display_cursor_col, display_cursor_row );
   if( DISPLAY_WIDTH == ++display_cursor_col ) {
      /* Overflow to the left and move to next row. */
      display_cursor_col = 0;
      if( DISPLAY_HEIGHT == ++display_cursor_row ) {
         display_cursor_row = 0;
      }
   }
}

void display_shutdown() {
}

