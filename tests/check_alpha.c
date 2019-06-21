
#include <check.h>
#include <stdint.h>

#include "../src/alpha.h"

#include <stdlib.h>

static const struct astring g_str_sentence =
   astring_l( "The quick brown fox jumped over the lazy dog!" );
static const struct astring g_str_the = astring_l( "The" );
static const struct astring g_str_quick = astring_l( "quick" );
static const struct astring g_str_brown = astring_l( "brown" );
static const struct astring g_str_fox = astring_l( "fox" );
static const struct astring g_str_jumped = astring_l( "jumped" );
static const struct astring g_str_over = astring_l( "over" );

static const struct astring g_str_list[4] = {
   astring_l( "Lorem" ),
   astring_l( "Ipsum" ),
   astring_l( "Sit" ),
   astring_l( "Amet" )
};

static const int g_str_list_lens[4] = {
   6, 6, 4, 5
};

static const int g_str_list_szs[4] = {
   6, 6, 4, 5
};

#include <stdio.h>

START_TEST( test_alpha_astr_len ) {
   int i = 0;
   const struct astring* test = g_str_list;

   for( i = 0 ; _i > i ; i++ ) {
      uint8_t* cp = (uint8_t*)test;
      int j = 0;
      int max = sizeof( struct astring ) + test->len;
      for( j = 0 ; max > j ; j++ ) {
         printf( "%d - %c\n", j, cp[j] );
      }

      test = alpha_astring_list_next( test );
   }

   ck_assert_int_eq( test->len, g_str_list_lens[_i] );
}
END_TEST

START_TEST( test_alpha_tok_cmp_c ) {
   const char* c = NULL;
   c = alpha_tok( &g_str_sentence, ' ', _i );
   ck_assert_int_eq( 0, alpha_cmp_c( "The", 3, &g_str_sentence, ' ' ) );
   switch( _i ) {
      case 0:
         ck_assert_int_eq( 0, alpha_cmp_c( c, 3, &g_str_the, ' ' ) );
         ck_assert_int_ne( 0, alpha_cmp_c( c, 5, &g_str_quick, ' ' ) );
         break;
      case 1:
         ck_assert_int_ne( 0, alpha_cmp_c( c, 3, &g_str_the, ' ' ) );
         ck_assert_int_eq( 0, alpha_cmp_c( c, 5, &g_str_quick, ' ' ) );
         ck_assert_int_ne( 0, alpha_cmp_c( c, 5, &g_str_brown, ' ' ) );
         break;
      case 2:
         ck_assert_int_ne( 0, alpha_cmp_c( c, 5, &g_str_quick, ' ' ) );
         ck_assert_int_eq( 0, alpha_cmp_c( c, 5, &g_str_brown, ' ' ) );
         ck_assert_int_ne( 0, alpha_cmp_c( c, 3, &g_str_fox, ' ' ) );
         break;
      case 3:
         ck_assert_int_ne( 0, alpha_cmp_c( c, 5, &g_str_brown, ' ' ) );
         ck_assert_int_eq( 0, alpha_cmp_c( c, 3, &g_str_fox, ' ' ) );
         ck_assert_int_ne( 0, alpha_cmp_c( c, 6, &g_str_jumped, ' ' ) );
         break;
      case 4:
         ck_assert_int_ne( 0, alpha_cmp_c( c, 3, &g_str_fox, ' ' ) );
         ck_assert_int_eq( 0, alpha_cmp_c( c, 6, &g_str_jumped, ' ' ) );
         ck_assert_int_ne( 0, alpha_cmp_c( c, 4, &g_str_over, ' ' ) );
         break;
   }
}
END_TEST

START_TEST( test_alpha_cmp_c ) {
   ck_assert_int_eq( 0, alpha_cmp_c( "The", 3, &g_str_sentence, ' ' ) );
}
END_TEST

START_TEST( test_alpha_tok ) {
   const char* c = NULL;
   c = alpha_tok( &g_str_sentence, ' ', _i );
   switch( _i ) {
      case 0:
         ck_assert_str_le( "The", c );
         break;
      case 1:
         ck_assert_str_le( "quick", c );
         break;
      case 2:
         ck_assert_str_le( "brown", c );
         break;
      case 3:
         ck_assert_str_le( "fox", c );
         break;
      case 4:
         ck_assert_str_le( "jumped", c );
         break;
   }
}
END_TEST

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

   tcase_add_loop_test( tc_core, test_alpha_astr_len, 0, 4 );
   tcase_add_test( tc_core, test_alpha_udigits );
   tcase_add_test( tc_core, test_alpha_udigits_hex );
   tcase_add_test( tc_core, test_alpha_utoa );
   tcase_add_test( tc_core, test_alpha_utoa_hex );
   //tcase_add_test( tc_core, test_alpha_insert );
   tcase_add_loop_test( tc_core, test_alpha_tok, 0, 4 );
   tcase_add_test( tc_core, test_alpha_cmp_c );
   tcase_add_loop_test( tc_core, test_alpha_tok_cmp_c, 0, 4 );

   suite_add_tcase( s, tc_core );

   return s;
}

