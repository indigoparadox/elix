
#include "cstd.h"

#define assert( t ) \
   if( !(t) ) { \
      printf( "bad assert on " __FILE__ " line %d\n", __LINE__ ); \
      return 1; \
   }

int putchar( int c ) {
   int retval;
   long len;
   char* pc = (char*)&c;

   len = 1;

   asm volatile (
      "movq $0x01, %%rax\n\t"
      "movq $0x01, %%rdi\n\t"
      "movq %1, %%rsi\n\t"
      "movl %2, %%edx\n\t"
      "syscall\n"
      : "=g"( retval )
      : "g" ( pc ), "g" ( len ) );
   return retval;
}

void exit( int status ) {
   long syscall = 1;
   asm volatile (
      "int $0x80\n"
      :
      : "a" ( syscall ), "b" ( status )
      : "memory" );
}

int main() {
   char str_test[31] = "This is a test.";
   char* s = "test";
   char* r = "replacement";
   int len = 0;
   int i = 0;

   putchar( 'f' );
   putchar( '\n' );

   for( i = 0 ; strlen( r ) > i ; i++ ) {
      putchar( r[i] );
   }
   putchar( '\n' );

   len = strlen( s );
   assert( len == 4 );
   printf( "Passed: strlen( s ): %d\n", len );
   assert( strlen( r ) == 11 );
   printf( "Passed: strlen( r ): %d\n", strlen( r ) );
   assert( strlen( str_test ) < 30 );
   printf( "Passed: strlen( str_test ): %d\n", strlen( str_test ) );

   printf( "strncmp( \"Foo\", \"Fii\", 3 ): %d\n",
      strncmp( "Foo", "Fii", 3 ) );
   assert( strncmp( "Foo", "Fii", 3 ) > 0 );

   printf( "strncmp( \"Faa\", \"Fii\", 3 )\n" );
   assert( strncmp( "Faa", "Fii", 3 ) < 0 );

   assert( strncmp( "Grumpy", "Gru", 3 ) == 0 );
   printf( "Passed: strncmp( \"Grumpy\", \"Gru\", 3 )\n" );

   printf( "Before replace: %s\n", str_test );

   strnreplace( str_test, 30, s, r );

   printf( "After replace: %s\n", str_test );

   return 0;
}

void _start() {
   int retval;

   retval = main();

   exit( retval );
}
