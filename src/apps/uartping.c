
#include "../kernel.h"
#include "../alpha.h"
#include "../adhd.h"

const struct astring g_str_uart = astring_l( "uarp" );

TASK_RETVAL uartping_task() {
   static uint8_t c = 'a';

   adhd_task_setup();

   adhd_set_gid( &g_str_uart );
      
   if( 'z' <= c ) {
      c = 'a';
   }
   uart_putc( 1, c++ );

   adhd_yield();
   adhd_end_loop();
}

