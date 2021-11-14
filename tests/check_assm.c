
#include <check.h>

const char* gc_test_in = "";

START_TEST( test_assm_encode_single ) {
}
END_TEST

Suite* assm_suite( void ) {
   Suite* s;
   TCase* tc_core;

   s = suite_create( "assm" );

   /* Core test case */
   tc_core = tcase_create( "Core" );

   tcase_add_loop_test( tc_core, test_assm_encode_single, 0, 4 );

   suite_add_tcase( s, tc_core );

   return s;
}

