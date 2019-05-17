
#include <check.h>
#include <stdint.h>
#include "../src/alpha.h"

START_TEST( test_alpha_insert ) {
}
END_TEST

Suite* alpha_suite( void ) {
   Suite* s;
   TCase* tc_core;

   s = suite_create( "alpha" );

   /* Core test case */
   tc_core = tcase_create( "Core" );

   tcase_add_test( tc_core, test_alpha_insert );

   suite_add_tcase( s, tc_core );

   return s;
}

