
#include "../keyboard.h"

#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>

//#include <ncurses.h>

#define STDIN 0

void keyboard_init() {
   struct termios term;
   tcgetattr( STDIN, &term );
   term.c_lflag &= ~ICANON;
   tcsetattr( STDIN, TCSANOW, &term );
   setbuf( stdin, NULL );
}

int keyboard_hit() {
   struct timeval timeout;
   fd_set rdset;
   FD_ZERO( &rdset );
   FD_SET( STDIN, &rdset );
   timeout.tv_sec = 0;
   timeout.tv_usec = 0;
   return select( STDIN + 1, &rdset, NULL, NULL, &timeout );
}

unsigned char keyboard_getc() {
   /* unsigned char buffer = 0;
   if( 1 != read( &buffer, 1 ) ) {
      return 0;
   } */
   return getchar();
}

