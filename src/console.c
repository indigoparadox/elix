
#include "code16.h"

#define CONSOLE_C
#include "console.h"
#include "display.h"
#include "io.h"

#define TPRINT_PAD_ZERO 0
#define TPRINT_PAD_SPACE 1

void abort() {
}

static void tpad( char pad, STRLEN_T len ) {
   uint8_t i = 0;

   if( 0 >= len || '\0' == pad ) {
      return;
   }

   while( 0 == len || i < len) {
      tputc( pad );
      i++;
   }
}

