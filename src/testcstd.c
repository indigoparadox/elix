
#include "cstd.h"

#include <assert.h>

int main() {
   char str_test[30] = "This is a test.";
   char* s = "test";
   char* r = "foo";

   assert( strlen( s ) == 4 );
   assert( strlen( r ) == 3 );
   assert( strlen( str_test ) < 30 );
   printf( "%d\n", strlen( str_test ) );
   return 0;

   printf( "%s\n", str_test );

   strnreplace( str_test, 30, s, r );

   printf( "%s\n", str_test );

   return 0;
}

