
#include <check.h>
#include <stdint.h>
#include <stdio.h>

#include "../src/mfat.h"

START_TEST( test_mfat_fat_sz ) {
}
END_TEST

Suite* mfat_suite( void ) {
   Suite* s;
   TCase* tc_core;

   s = suite_create( "mfat" );

   /* Core test case */
   tc_core = tcase_create( "Core" );

   //tcase_add_loop_test( tc_core, test_mfat_, 0, 4 );
   tcase_add_test( tc_core, test_mfat_fat_sz );

   suite_add_tcase( s, tc_core );

   return s;
}

