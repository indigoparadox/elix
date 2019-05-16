
#include "../keyboard.h"
#include "../kernel.h"

#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <signal.h>

extern uint8_t g_system_state;

void handle_ctrl_c( int param ) {
   tputs( "Shutting down...\n" );
   /* Tell the kernel we can exit. */
   g_system_state = SYSTEM_SHUTDOWN;
}

void keyboard_init() {
   struct termios term;

   /* Handle CTRL-C. */
   signal( SIGINT, handle_ctrl_c );

   tcgetattr( STDIN, &term );
   term.c_lflag &= ~ICANON;
   tcsetattr( STDIN, TCSANOW, &term );

   setbuf( stdin, NULL );
}

void keyboard_shutdown() {
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

