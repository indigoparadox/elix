
#include "../display.h"
#include "../kernel.h"
#include "../mem.h"
#include "../io.h"

#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>

static uint8_t g_cur_pos = 0;

void display_set_colors( uint8_t fg, uint8_t bg ) {
}

void display_init() {
   struct termios term;

   io_regoutdev( display_putc );

   /* Disable echo. */
   tcgetattr( STDIN, &term );
   term.c_lflag &= ~ECHO;
   tcsetattr( STDIN, TCSANOW, &term );
}

void display_shutdown() {
   struct termios term;

   /* Re-enable echo. */
   tcgetattr( STDIN, &term );
   term.c_lflag |= ECHO;
   tcsetattr( STDIN, TCSANOW, &term );
}

void display_putc_at( char c, int x, int y ) {
}

/* Put a character at the cursor's current spot and shift the cursor right by
 * one. If the cursor is past the edge of the screen, move to a new line.
 */
void display_putc( char c ) {
   if( DISPLAY_WIDTH <= g_cur_pos ) {
      printf( "\n" );
      g_cur_pos = 0;
   }
   if( '\n' == c ) {
      g_cur_pos = 0;
   }
   printf( "%c", c );
}

