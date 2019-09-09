
#include "../code16.h"

#include "../kernel.h"

char cbuffer = '\0';

void keyboard_init() {
}

void keyboard_shutdown() {
}

int keyboard_hit() {
   char out = '\0';
   __asm__ __volatile__ ("int $0x16" : "=a"(out) : "0"(0x0));
   if( '\0' != out ) {
      cbuffer = out;
      return 1;
   }
   return 0;
}

char keyboard_getc() {
   char out = '\0';
   if( '\0' != cbuffer ) {
      out = cbuffer;
      cbuffer = '\0';
   }
   return out;
}

