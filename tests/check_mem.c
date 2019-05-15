
#include <check.h>
#include <stdint.h>
#include "../src/mem.h"

#define CHECK_PID 1

#define CHECK_MID_TEST1 1 
#define CHECK_MID_TEST2 2

#define CHECK_MEM_SHIFT_OFFSET 5

#define CHECK_STR_SZ 6
#define CHECK_STR_SZ_2 9
uint8_t g_chk_str_1[CHECK_STR_SZ] = { 0x01, 0x12, 0x23, 0x34, 0x45, 0x56 };
uint8_t g_chk_str_2[CHECK_STR_SZ] = { 0xdd, 0xee, 0xaa, 0xdb, 0xee, 0xff };
uint8_t g_chk_str_3[CHECK_STR_SZ_2] = { 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2 };

extern uint8_t g_mheap[MEM_HEAP_SIZE];
extern int g_mheap_top;

START_TEST( test_mset ) {
   int i = 0;
   int offset = 0;
   int test_heap = 0;
   int test_sample = 0;
   struct mvar* var_header = NULL;

   minit();

   mset( CHECK_PID, CHECK_MID_TEST1, g_chk_str_1, CHECK_STR_SZ );

   var_header = (struct mvar*)&(g_mheap[i]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ + 1 );
   offset += sizeof( struct mvar );
   for( i = offset ; CHECK_STR_SZ > i ; i++ ) {
      test_heap = g_mheap[i];
      test_sample = g_chk_str_1[i];
      ck_assert_int_eq( test_heap, test_sample );
   }
   ck_assert_int_eq( g_mheap_top, sizeof( struct mvar ) + CHECK_STR_SZ + 1 );
}
END_TEST

START_TEST( test_mshift ) {
   struct mvar* var_header = NULL;

   minit();

   mset( CHECK_PID, CHECK_MID_TEST1, g_chk_str_1, CHECK_STR_SZ );

   //mprint();

   mshift( 0, CHECK_MEM_SHIFT_OFFSET );

   //mprint();

   var_header = &(g_mheap[CHECK_MEM_SHIFT_OFFSET]);  

   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ + 1 );
   
}
END_TEST

START_TEST( test_mset_heap ) {
   int i = 0;
   int offset = 0;
   int test_heap = 0;
   int test_sample = 0;
   int correct_heap_top = 0;
   struct mvar* var_header = NULL;

   minit();

   //mprint();

   mset( CHECK_PID, CHECK_MID_TEST1, g_chk_str_1, CHECK_STR_SZ );
   
   //mprint();

   mset( CHECK_PID, CHECK_MID_TEST2, g_chk_str_2, CHECK_STR_SZ );

   //mprint();

   correct_heap_top =
      (2 * sizeof( struct mvar )) + /* Both headers. */
      2 +                           /* Both NULLs */
      CHECK_STR_SZ + CHECK_STR_SZ;

   ck_assert_int_eq( g_mheap_top, correct_heap_top );

   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ + 1 );
   offset += sizeof( struct mvar );
   for( i = offset ; CHECK_STR_SZ > i ; i++ ) {
      test_heap = g_mheap[i];
      test_sample = g_chk_str_1[i];
      ck_assert_int_eq( test_heap, test_sample );
   }

   /* Advance past the first var. */
   offset += CHECK_STR_SZ;
   offset += 1; /* For term NULL. */

   /* Check the next var. */
   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST2 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ + 1 );
   offset += sizeof( struct mvar );
   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      test_heap = g_mheap[offset + i];
      test_sample = g_chk_str_2[i];
      ck_assert_int_eq( test_heap, test_sample );
   }
}
END_TEST

START_TEST( test_mset_heap_overwrite ) {
   int i = 0;
   int offset = 0;
   int test_heap = 0;
   int test_sample = 0;
   int correct_heap_top = 0;
   struct mvar* var_header = NULL;

   minit();

   //mprint();

   mset( CHECK_PID, CHECK_MID_TEST1, g_chk_str_1, CHECK_STR_SZ );
   
   //mprint();

   mset( CHECK_PID, CHECK_MID_TEST2, g_chk_str_2, CHECK_STR_SZ );

   mprint();

   mset( CHECK_PID, CHECK_MID_TEST1, g_chk_str_3, CHECK_STR_SZ_2 );

   mprint();

   correct_heap_top =
      (2 * sizeof( struct mvar )) + /* Both headers. */
      2 +                           /* Both NULLs */
      CHECK_STR_SZ + CHECK_STR_SZ_2;

   ck_assert_int_eq( g_mheap_top, correct_heap_top );

   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ_2 + 1 );
   offset += sizeof( struct mvar );
   for( i = 0 ; CHECK_STR_SZ_2 > i ; i++ ) {
      test_heap = g_mheap[i + offset];
      test_sample = g_chk_str_3[i];
      ck_assert_int_eq( test_heap, test_sample );
   }

   /* Advance past the first var. */
   offset += CHECK_STR_SZ_2;
   offset += 1; /* For term NULL. */

   /* Check the next var. */
   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST2 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ + 1 );
   offset += sizeof( struct mvar );
   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      test_heap = g_mheap[offset + i];
      test_sample = g_chk_str_2[i];
      ck_assert_int_eq( test_heap, test_sample );
   }
}
END_TEST

START_TEST( test_mget ) {
   uint8_t* chk_mem_ret_1 = NULL;
   int chk_mem_len_1 = 0;
   int i = 0;

   minit();
   mset( CHECK_PID, CHECK_MID_TEST1, g_chk_str_1, CHECK_STR_SZ );

   chk_mem_ret_1 = mget( CHECK_PID, CHECK_MID_TEST1, &chk_mem_len_1 );

   ck_assert_int_eq( chk_mem_len_1, CHECK_STR_SZ );
   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      ck_assert_int_eq( chk_mem_ret_1[i], g_chk_str_1[i] );
   }

}
END_TEST

Suite* mem_suite( void ) {
   Suite* s;
   TCase* tc_core;

   s = suite_create( "mem" );

   /* Core test case */
   tc_core = tcase_create( "Core" );

   tcase_add_test( tc_core, test_mset );
   tcase_add_test( tc_core, test_mshift );
   tcase_add_test( tc_core, test_mset_heap );
   tcase_add_test( tc_core, test_mset_heap_overwrite );
   tcase_add_test( tc_core, test_mget );

   suite_add_tcase( s, tc_core );

   return s;
}


