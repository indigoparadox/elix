
#include "../display.h"
#include "../kernel.h"
#include "../mem.h"

#include <sys/select.h>
#ifdef COLINUX_TERMIOS
#include <termios.h>
#include <stdio.h>
#elif defined( COLINUX_CURSES )
#include <ncurses.h>
#elif defined( COLINUX_READLINE )
#include <stdio.h>
#endif /* COLINUX_TERMIOS || COLINUX_CURSES */
#include <stropts.h>

#if defined( COLINUX_TERMIOS ) || defined( COLINUX_READLINE )
static uint8_t g_cur_pos = 0;
#endif /* COLINUX_TERMIOS || COLINUX_CURSES || COLINUX_READLINE */

void display_set_colors( uint8_t fg, uint8_t bg ) {
}

void display_init() {
#ifdef COLINUX_TERMIOS
   struct termios term;
#endif /* COLINUX_TERMIOS */

#ifdef COLINUX_TERMIOS
   /* Disable echo. */
   tcgetattr( STDIN, &term );
   term.c_lflag &= ~ECHO;
   tcsetattr( STDIN, TCSANOW, &term );
   setbuf( stdin, NULL );
#elif defined( COLINUX_CURSES )
   initscr();
   /* cbreak();
   nodelay( stdscr, true ); */
   noecho();
   raw();
#endif /* COLINUX_TERMIOS || COLINUX_CURSES */
}

void display_shutdown() {
#ifdef COLINUX_TERMIOS
   struct termios term;

   /* Re-enable echo. */
   tcgetattr( STDIN, &term );
   term.c_lflag |= ECHO;
   tcsetattr( STDIN, TCSANOW, &term );
#elif defined( COLINUX_CURSES )
   endwin();
#endif /* COLINUX_TERMIOS || COLINUX_CURSES */
}

void display_putc_at( char c, int x, int y ) {
}

/* Put a character at the cursor's current spot and shift the cursor right by
 * one. If the cursor is past the edge of the screen, move to a new line.
 */
void display_putc( uint8_t dev_index, char c ) {
#if defined( COLINUX_TERMIOS ) || defined( COLINUX_READLINE )
   if( DISPLAY_WIDTH <= g_cur_pos ) {
      printf( "\n" );
      g_cur_pos = 0;
   }
   if( '\n' == c ) {
      g_cur_pos = 0;
   }
   printf( "%c", c );
   fflush( stdout );
#elif defined( COLINUX_CURSES )
   //printw( "%c", c );
   //refresh();
#endif /* COLINUX_TERMIOS || COLINUX_CURSES || COLINUX_READLINE */
}

