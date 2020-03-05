
#include "cstd.h"

#define assert( t ) \
   if( !(t) ) { \
      printf( "bad assert on " __FILE__ " line %d\n", __LINE__ ); \
   }

int main() {
   char str_test[31] = "This is a test.";
   char* s = "test";
   char* r = "replacement";

   assert( strlen( s ) == 4 );
   printf( "Passed: strlen( s ): %d\n", strlen( s ) );
   fflush( 0 );
   assert( strlen( r ) == 11 );
   printf( "Passed: strlen( r ): %d\n", strlen( r ) );
   fflush( 0 );
   assert( strlen( str_test ) < 30 );
   printf( "Passed: strlen( str_test ): %d\n", strlen( str_test ) );
   fflush( 0 );

   printf( "Before replace: %s\n", str_test );

   strnreplace( str_test, 30, s, r );

   printf( "After replace: %s\n", str_test );

   return 0;
}

