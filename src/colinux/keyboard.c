
#include "../kernel.h"
#include "../io.h"

#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#ifdef COLINUX_TERMIOS
#include <sys/select.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#elif defined( COLINUX_CURSES )
#include <ncurses.h>
#elif defined( COLINUX_READLINE )
#include <readline/readline.h>
#endif /* COLINUX_TERMIOS || COLINUX_CURSES || COLINUX_READLINE */
#include <stropts.h>
#include <signal.h>

#ifdef COLINUX_READLINE
#define COLINUX_READLINE_CHAR_STACK_SZ 100
volatile int g_char_stack[COLINUX_READLINE_CHAR_STACK_SZ];
volatile int g_char_stack_top = 0;

static int handle_key( int count, int key ) {
   if( g_char_stack_top + 1 >= COLINUX_READLINE_CHAR_STACK_SZ ) {
      return 0;
   }
   g_char_stack[g_char_stack_top] = key;
   g_char_stack_top++;
   return 0;
}
#endif /* COLINUX_READLINE */

static void handle_ctrl_c( int param ) {
   /* Tell the kernel we can exit. */
   g_system_state = SYSTEM_SHUTDOWN;
}

char keyboard_hit() {
   int bytes = 0;
#ifdef COLINUX_TERMIOS
/*
   struct timeval timeout;
   fd_set rdset;
   FD_ZERO( &rdset );
   FD_SET( STDIN, &rdset );
   timeout.tv_sec = 0;
   timeout.tv_usec = 0;
   return select( STDIN + 1, &rdset, NULL, NULL, &timeout );
*/
   ioctl( STDIN, FIONREAD, &bytes );
#elif defined( COLINUX_CURSES )
   //bytes = getch();
#elif defined( COLINUX_READLINE )
   bytes = g_char_stack_top;
#endif /* COLINUX_TERMIOS || COLINUX_CURSES */
   return (char)bytes;
}

char keyboard_getc() {
   char buffer = 0;
   /* if( 1 != read( &buffer, 1 ) ) {
      return 0;
   } */
#ifdef COLINUX_TERMIOS
   /*buffer = getchar();*/
   read( STDIN_FILENO, &buffer, 1 );
#elif defined( COLINUX_CURSES )
   buffer = getch();
#elif defined( COLINUX_READLINE )
   int i = 0;
   buffer = g_char_stack[0];
   for( i = 0 ; g_char_stack_top > i ; i++ ) {
      g_char_stack[i] = g_char_stack[i + 1];
   }
#endif /* COLINUX_TERMIOS || COLINUX_CURSES || COLINUX_READLINE */
   return buffer;
}

//__attribute__( (constructor( CTOR_PRIO_DISPLAY ) ) )
void keyboard_init() {
#ifdef COLINUX_TERMIOS
   struct termios term;
#elif defined( COLINUX_CURSES )
#endif /* COLINUX_TERMIOS || COLINUX_CURSES */

   /* Handle CTRL-C. */
   signal( SIGINT, handle_ctrl_c );

#ifdef COLINUX_TERMIOS
   tcgetattr( STDIN, &term );

   term.c_lflag &= ~ICANON;
   term.c_lflag &= ~ECHO;
   tcsetattr( STDIN, TCSANOW, &term );

   setbuf( stdin, NULL );
#elif defined( COLINUX_CURSES )
#elif defined( COLINUX_READLINE )
   rl_bind_key(
#endif /* COLINUX_TERMIOS || COLINUX_CURSES || COLINUX_READLINE */
}

__attribute__( (destructor( CTOR_PRIO_DISPLAY )) )
static void keyboard_shutdown() {
}

