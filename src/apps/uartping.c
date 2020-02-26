
#include "../kernel.h"
#include "../alpha.h"
#include "../adhd.h"

TASK_RETVAL uartping_task() {
   static uint8_t c = 'a';

   adhd_task_setup();

   if( 'z' <= c ) {
      c = 'a';
   }
   uart_putc( 1, c++ );

   adhd_yield();
   adhd_end_loop();
}

