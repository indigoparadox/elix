
#include "mem.h"

int mcopy( void* dest, const void* src, int sz ) {
   int i = 0;
   for( i = 0 ; sz > i ; i++ ) {
      ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
   }
   return i;
}

int mcompare( const void* c1, const void* c2, int sz ) {
   int i = 0;
   for( i = 0 ; sz > i ; i++ ) {
      if( ((uint8_t*)c1)[i] != ((uint8_t*)c2)[i] ) {
         return 1;
      }
   }
   return 0;
}

int mstrlen( const char* str ) {
   int i = 0;
   while( '\0' != str[i] ) { i++; }
   return i;
}

/*
struct mstring* mfromcstr( char* str ) {
   char* c = str;
   int len = 0;
   while( '\0' != *c ) {
      c++;
      len++;
   }
}
*/

