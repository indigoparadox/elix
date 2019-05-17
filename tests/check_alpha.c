
#include <check.h>
#include <stdint.h>

#include "../src/alpha.h"

#include <stdlib.h>

START_TEST( test_alpha_udigits ) {
   ck_assert_int_eq( 4, alpha_udigits( 1234, 10 ) );
   ck_assert_int_eq( 3, alpha_udigits( 123, 10 ) );
   ck_assert_int_eq( 2, alpha_udigits( 12, 10 ) );
   ck_assert_int_eq( 1, alpha_udigits( 1, 10 ) );
}
END_TEST

START_TEST( test_alpha_udigits_hex ) {
   ck_assert_int_eq( 4, alpha_udigits( 0x1234, 16 ) );
   ck_assert_int_eq( 3, alpha_udigits( 0x123, 16 ) );
   ck_assert_int_eq( 2, alpha_udigits( 0x12, 16 ) );
   ck_assert_int_eq( 1, alpha_udigits( 0x1, 16 ) );
}
END_TEST

START_TEST( test_alpha_utoa ) {
   struct astring* buffer = NULL;
   int test_int = 12345;
   int16_t len = 0;

   buffer = calloc( 1, sizeof( struct astring ) + INT_DIGITS_MAX );
   buffer->sz = INT_DIGITS_MAX;

   len = alpha_utoa( test_int, buffer, 0, 0, 10 );

   ck_assert_int_eq( len, alpha_udigits( test_int, 10 ) );
   ck_assert_str_eq( buffer->data, "12345" );

   free( buffer );
}
END_TEST

START_TEST( test_alpha_utoa_hex ) {
   struct astring* buffer = NULL;
   int test_int = 0x2345;
   int16_t len = 0;

   buffer = calloc( 1, sizeof( struct astring ) + INT_DIGITS_MAX );
   buffer->sz = INT_DIGITS_MAX;

   len = alpha_utoa( test_int, buffer, 0, 0, 16 );

   ck_assert_int_eq( len, alpha_udigits( test_int, 16 ) );
   ck_assert_str_eq( buffer->data, "2345" );

   free( buffer );
}
END_TEST

/*
START_TEST( test_alpha_insert ) {
}
END_TEST
*/

Suite* alpha_suite( void ) {
   Suite* s;
   TCase* tc_core;

   s = suite_create( "alpha" );

   /* Core test case */
   tc_core = tcase_create( "Core" );

   tcase_add_test( tc_core, test_alpha_udigits );
   tcase_add_test( tc_core, test_alpha_udigits_hex );
   tcase_add_test( tc_core, test_alpha_utoa );
   tcase_add_test( tc_core, test_alpha_utoa_hex );
   //tcase_add_test( tc_core, test_alpha_insert );

   suite_add_tcase( s, tc_core );

   return s;
}

