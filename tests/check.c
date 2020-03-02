
#include <check.h>

#define IO_C
#include "../src/io.h"

#define main_add_test_proto( suite_name ) \
   Suite* suite_name ## _suite();

#define main_add_test( suite_name ) \
   Suite* s_ ## suite_name = suite_name ## _suite(); \
   SRunner* sr_ ## suite_name = srunner_create( s_ ## suite_name ); \
   /* srunner_set_fork_status( sr_ ## suite_name, CK_NOFORK ); */ \
   srunner_run_all( sr_ ## suite_name, CK_VERBOSE ); \
   number_failed += srunner_ntests_failed( sr_ ## suite_name ); \
   srunner_free( sr_ ## suite_name );

/* main_add_test_proto( mem )
main_add_test_proto( console )
main_add_test_proto( alpha ) */
main_add_test_proto( cstd )

#define SYSTEM_RUNNING 0
uint8_t g_system_state = SYSTEM_RUNNING;

int main( void ) {
   int number_failed = 0;

   /* main_add_test( mem );
   main_add_test( console );
   main_add_test( alpha ); */
   main_add_test( cstd );

   return( number_failed == 0 ) ? 0 : 1;
}

