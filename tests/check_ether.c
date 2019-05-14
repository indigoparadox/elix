
#include <check.h>
#include <stdint.h>
#include "../src/net/ether.h"

uint8_t g_src_mac[6] = { 0x01, 0x12, 0x23, 0x34, 0x45, 0x56 };
uint8_t g_bcast_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

START_TEST( test_ether_htons ) {
   uint16_t test_val = 0x1234;
   
   test_val = ether_htons( test_val );

   ck_assert_int_eq( test_val, 0x3412 );

   test_val = ether_ntohs( test_val );

   ck_assert_int_eq( test_val, 0x1234 );
}
END_TEST

START_TEST( test_ether_new_frame ) {
   struct ether_frame* frame = NULL;
   enum ether_type type = ETHER_TYPE_ARP;
   int i = 0;
   uint8_t test_data[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

   frame = ether_new_frame( g_src_mac, g_bcast_mac, type, test_data, 10 );

   ck_assert_int_eq( frame->header.type, ether_ntohs( (uint16_t)type ) );

   for( i = 0 ; 6 > i ; i++ ) {
      ck_assert_int_eq( frame->header.src_mac[i], g_src_mac[i] );
   }

   for( i = 0 ; 6 > i ; i++ ) {
      ck_assert_int_eq( frame->header.dest_mac[i], g_bcast_mac[i] );
   }

   for( i = 0 ; 10 > i ; i++ ) {
      ck_assert_int_eq( frame->data[i], test_data[i] );
   }

   //free( frame );
}
END_TEST

Suite* ether_suite( void ) {
   Suite* s;
   TCase* tc_core;

   s = suite_create( "ether" );

   /* Core test case */
   tc_core = tcase_create( "Core" );

   tcase_add_test( tc_core, test_ether_htons );
   tcase_add_test( tc_core, test_ether_new_frame );

   suite_add_tcase( s, tc_core );

   return s;
}


