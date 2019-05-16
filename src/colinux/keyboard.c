
#include "../keyboard.h"

#include <sys/ioctl.h>
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

uint8_t keyboard_hit() {
   int waiting;
   ioctl( STDIN, FIONREAD, &waiting );
   return (uint8_t)waiting;
}

unsigned char keyboard_getc() {
   unsigned char buffer = 0;
   if( 1 != read( &buffer, 1 ) ) {
      return 0;
   }
   return buffer;
}

