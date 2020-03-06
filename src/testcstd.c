
#include "cstd.h"

#define cstd_assert_str( s ) #s
#define cstd_assert( t, ... ) \
   if( !(t) ) { \
      printf( "failed: bad assert on " __FILE__ " line %d\n", __LINE__ ); \
      return 1; \
   } else { \
      printf( "passed: " cstd_assert_str( t ) ": " __VA_ARGS__ ); \
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
   char str_tok_test[30] = "Testing tokens now.";
   char* tok_hold = NULL;
   char* t = NULL;
   char str_repl_samelen_test[31] = "This is a tst.";
   char* s_same = "tst";
   char* r_same = "foo";

   /* Test: putchar (needed for other tests) */
   for( i = 0 ; strlen( r ) > i ; i++ ) {
      putchar( r[i] );
   }
   putchar( '\n' );

   /* Test: strlen */
   len = strlen( s );
   cstd_assert( len == 4, "%d\n", len );
   cstd_assert( strlen( r ) == 11, "%d\n", strlen( r ) );
   cstd_assert( strlen( str_test ) < 30, "%d\n", strlen( str_test ) );

   /* Test: strncmp */
   cstd_assert( strncmp( "Foo", "Fii", 3 ) > 0, "%d\n",
      strncmp( "Foo", "Fii", 3 ) );

   cstd_assert( strncmp( "Faa", "Fii", 3 ) < 0, "%d\n",
      strncmp( "Foo", "Fii", 3 ) );

   cstd_assert( strncmp( "Grumpy", "Gru", 3 ) == 0, "%d\n",
      strncmp( "Grumpy", "Gru", 3 ) );

   /* Test: strtok */
   t = strtok_r( str_tok_test, " ", &tok_hold );
   cstd_assert( strncmp( t, "Testing", 7 ) == 0, "%d\n",
      strncmp( t, "Testing", 7 ) );

   t = strtok_r( NULL, " ", &tok_hold );
   cstd_assert( strncmp( t, "tokens", 6 ) == 0, "%d\n",
      strncmp( t, "tokens", 6 ) );

   t = strtok_r( NULL, " ", &tok_hold );
   cstd_assert( strncmp( t, "now.", 4 ) == 0, "%d\n",
      strncmp( t, "now.", 4 ) );

   t = strtok_r( NULL, " ", &tok_hold );
   cstd_assert( NULL == t, "NULL == t\n" );

   strnreplace( str_tok_test, 30, NULL, " " );
   cstd_assert( strncmp( str_tok_test, "Testing tokens now.", 19 ) == 0, "%d\n",
      strncmp( str_tok_test, "Testing tokens now.", 19 ) );

   /* Test: strnreplace */
   printf( "Before replace: %s\n", str_test );
   strnreplace( str_test, 30, s, r );
   printf( "After replace: %s\n", str_test );

   printf( "Before replace: %s\n", str_repl_samelen_test );
   strnreplace( str_repl_samelen_test, 30, s_same, r_same );
   printf( "After replace: %s\n", str_repl_samelen_test );

   return 0;
}

void _start() {
   int retval;

   retval = main();

   exit( retval );
}

