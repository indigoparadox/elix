
#include <check.h>
#include <stdint.h>
#include "../src/mem.h"
#include "../src/console.h"

#define CHECK_PID 1

#define CHECK_MID_TEST1 1 
#define CHECK_MID_TEST2 2
#define CHECK_MID_LINE1 3
#define CHECK_MID_CUR_POS1 4

#define CHECK_MEM_SHIFT_OFFSET 5

#define CHECK_STR_SZ 6
#define CHECK_STR_SZ_2 9
uint8_t g_chk_str_1[CHECK_STR_SZ] = { 0x01, 0x12, 0x23, 0x34, 0x45, 0x56 };
uint8_t g_chk_str_2[CHECK_STR_SZ] = { 0xdd, 0xee, 0xaa, 0xdb, 0xee, 0xff };
uint8_t g_chk_str_3[CHECK_STR_SZ_2] = { 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2 };

extern uint8_t g_mheap[MEM_HEAP_SIZE];
extern int g_mheap_top;

START_TEST( test_mget ) {
   int i = 0;
   int offset = 0;
   int test_heap = 0;
   int test_sample = 0;
   struct mvar* var_header = NULL;

   minit();

   var_header = mget( CHECK_PID, CHECK_MID_TEST1, CHECK_STR_SZ );
   mcopy( var_header->data, g_chk_str_1, CHECK_STR_SZ );

   var_header = (struct mvar*)&(g_mheap[i]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ );
   offset += sizeof( struct mvar );
   for( i = offset ; CHECK_STR_SZ > i ; i++ ) {
      test_heap = g_mheap[i];
      test_sample = g_chk_str_1[i];
      ck_assert_int_eq( test_heap, test_sample );
   }
   ck_assert_int_eq( g_mheap_top, sizeof( struct mvar ) + CHECK_STR_SZ );
}
END_TEST

START_TEST( test_mshift ) {
   struct mvar* var_header = NULL;

   minit();

   var_header = mget( CHECK_PID, CHECK_MID_TEST1, CHECK_STR_SZ );
   mcopy( var_header->data, g_chk_str_1, CHECK_STR_SZ );

   //mprint();

   mshift( 0, CHECK_MEM_SHIFT_OFFSET );

   //mprint();

   var_header = (struct mvar*)&(g_mheap[CHECK_MEM_SHIFT_OFFSET]);  

   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ );
   
}
END_TEST

START_TEST( test_mget_heap ) {
   int i = 0;
   int offset = 0;
   int test_heap = 0;
   int test_sample = 0;
   int correct_heap_top = 0;
   struct mvar* var_header = NULL;

   minit();

   //mprint();

   var_header = mget( CHECK_PID, CHECK_MID_TEST1, CHECK_STR_SZ );
   mcopy( var_header->data, g_chk_str_1, CHECK_STR_SZ );
   
   //mprint();

   var_header = mget( CHECK_PID, CHECK_MID_TEST2, CHECK_STR_SZ );
   mcopy( var_header->data, g_chk_str_2, CHECK_STR_SZ );

   //mprint();

   correct_heap_top =
      (2 * sizeof( struct mvar )) + /* Both headers. */
      CHECK_STR_SZ + CHECK_STR_SZ;

   ck_assert_int_eq( g_mheap_top, correct_heap_top );

   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ );
   offset += sizeof( struct mvar );
   for( i = offset ; CHECK_STR_SZ > i ; i++ ) {
      test_heap = g_mheap[i];
      test_sample = g_chk_str_1[i];
      ck_assert_int_eq( test_heap, test_sample );
   }

   /* Advance past the first var. */
   offset += CHECK_STR_SZ;

   /* Check the next var. */
   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST2 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ );
   offset += sizeof( struct mvar );
   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      test_heap = g_mheap[offset + i];
      test_sample = g_chk_str_2[i];
      ck_assert_int_eq( test_heap, test_sample );
   }
}
END_TEST

START_TEST( test_mget_heap_overwrite ) {
   int i = 0;
   int offset = 0;
   int test_heap = 0;
   int test_sample = 0;
   int correct_heap_top = 0;
   struct mvar* var_header = NULL;

   minit();

   //mprint();

   var_header = mget( CHECK_PID, CHECK_MID_TEST1, CHECK_STR_SZ );
   mcopy( var_header->data, g_chk_str_1, CHECK_STR_SZ );
   
   //mprint();

   var_header = mget( CHECK_PID, CHECK_MID_TEST2, CHECK_STR_SZ );
   mcopy( var_header->data, g_chk_str_2, CHECK_STR_SZ );

   //mprint();

   var_header = mget( CHECK_PID, CHECK_MID_TEST1, CHECK_STR_SZ_2 );
   mcopy( var_header->data, g_chk_str_3, CHECK_STR_SZ_2 );

   //mprint();

   correct_heap_top =
      (2 * sizeof( struct mvar )) + /* Both headers. */
      CHECK_STR_SZ + CHECK_STR_SZ_2;

   ck_assert_int_eq( g_mheap_top, correct_heap_top );

   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ_2 );
   offset += sizeof( struct mvar );
   for( i = 0 ; CHECK_STR_SZ_2 > i ; i++ ) {
      test_heap = g_mheap[i + offset];
      test_sample = g_chk_str_3[i];
      ck_assert_int_eq( test_heap, test_sample );
   }

   /* Advance past the first var. */
   offset += CHECK_STR_SZ_2;

   /* Check the next var. */
   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST2 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ );
   offset += sizeof( struct mvar );
   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      test_heap = g_mheap[offset + i];
      test_sample = g_chk_str_2[i];
      ck_assert_int_eq( test_heap, test_sample );
   }
}
END_TEST

START_TEST( test_mget_pid_match ) {
   int i = 0;
   int offset = 0;
   int test_heap = 0;
   int test_sample = 0;
   int correct_heap_top = 0;
   struct mvar* var_header = NULL;

   //printf( "test_mset_pid_match\n" );

   minit();

   //mprint();

   var_header = mget( CHECK_PID + 1, CHECK_MID_TEST1, CHECK_STR_SZ );
   mcopy( var_header->data, g_chk_str_1, CHECK_STR_SZ );
   
   //mprint();

   var_header = mget( CHECK_PID, CHECK_MID_TEST2, CHECK_STR_SZ );
   mcopy( var_header->data, g_chk_str_2, CHECK_STR_SZ );

   //mprint();

   var_header = mget( CHECK_PID, CHECK_MID_TEST1, CHECK_STR_SZ_2 );
   mcopy( var_header->data, g_chk_str_3, CHECK_STR_SZ_2 );

   //mprint();

   correct_heap_top =
      (3 * sizeof( struct mvar )) + /* Both headers. */
      CHECK_STR_SZ + CHECK_STR_SZ + CHECK_STR_SZ_2;

   ck_assert_int_eq( g_mheap_top, correct_heap_top );

   /* Check the first var. */
   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_int_eq( var_header->pid, CHECK_PID + 1 );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ );
   offset += sizeof( struct mvar );
   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      test_heap = g_mheap[offset + i];
      test_sample = g_chk_str_1[i];
      ck_assert_int_eq( test_heap, test_sample );
   }

   //mprint();

   /* And check the mget block. */
   var_header = mget( CHECK_PID + 1, CHECK_MID_TEST1, CHECK_STR_SZ );
   ck_assert_int_eq( var_header->pid, CHECK_PID + 1 );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ );
   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      test_heap = var_header->data[i];
      test_sample = g_chk_str_1[i];
      ck_assert_int_eq( test_heap, test_sample );
   }

   /* Check the second var. */
   offset += CHECK_STR_SZ;

   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST2 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ );
   offset += sizeof( struct mvar );
   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      test_heap = g_mheap[offset + i];
      test_sample = g_chk_str_2[i];
      ck_assert_int_eq( test_heap, test_sample );
   }

   //mprint();

   /* And check the mget block. */
   var_header = mget( CHECK_PID, CHECK_MID_TEST2, CHECK_STR_SZ );
   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      ck_assert_int_eq( var_header->data[i], g_chk_str_2[i] );
   }

   /* Check the third var. */
   offset += CHECK_STR_SZ;

   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, CHECK_STR_SZ_2 );
   offset += sizeof( struct mvar );
   for( i = 0 ; CHECK_STR_SZ_2 > i ; i++ ) {
      test_heap = g_mheap[i + offset];
      test_sample = g_chk_str_3[i];
      ck_assert_int_eq( test_heap, test_sample );
   }

   //mprint();

   /* And check the mget block. */
   var_header = mget( CHECK_PID, CHECK_MID_TEST1, CHECK_STR_SZ_2 );
   for( i = 0 ; CHECK_STR_SZ_2 > i ; i++ ) {
      ck_assert_int_eq( var_header->data[i], g_chk_str_3[i] );
   }
}
END_TEST

#if 0
START_TEST( test_mget ) {
   uint8_t* chk_mem_ret_1 = NULL;
   unsigned int chk_mem_ret_2 = 0xc0fefeaa;
   int chk_mem_len_1 = 0;
   int i = 0;

   minit();

   mset( CHECK_PID, CHECK_MID_TEST1, g_chk_str_1, CHECK_STR_SZ );
   //mprint();
   mset( CHECK_PID, CHECK_MID_TEST2, &chk_mem_ret_2, sizeof( int ) );
   //mprint();

   chk_mem_ret_1 = mget( CHECK_PID, CHECK_MID_TEST1, &chk_mem_len_1 );

   ck_assert_int_eq( chk_mem_len_1, CHECK_STR_SZ );
   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      ck_assert_int_eq( chk_mem_ret_1[i], g_chk_str_1[i] );
   }

   chk_mem_ret_2 = mget_int( CHECK_PID, CHECK_MID_TEST2 );
   ck_assert_int_eq( chk_mem_ret_2, 0xc0fefeaa );
}
END_TEST

START_TEST( test_metazero ) {
   int m_not_found = 0;

   minit();

   m_not_found = mget_int( CHECK_PID, CHECK_MID_TEST1 );

   ck_assert_int_eq( m_not_found, 0 );
}
END_TEST

START_TEST( test_mset_line ) {
   int line_len = 0;
   uint8_t* cur_pos = NULL;
   int cur_pos_len = 0;
   char c = '\0';
   int i = 0;
   char* line = NULL;
   int heap_iter = 0;

   minit();

   /* Create the buffer and grab a pointer to it. */
   mset( CHECK_PID, CHECK_MID_CUR_POS1, NULL, sizeof( uint8_t ) );
   mset( CHECK_PID, CHECK_MID_LINE1, NULL, REPL_LINE_SIZE_MAX );
   cur_pos = mget( CHECK_PID, CHECK_MID_CUR_POS1, &cur_pos_len );
   line = mget( CHECK_PID, CHECK_MID_LINE1, &line_len );

   heap_iter = (2 * sizeof( struct mvar )) + 2 + REPL_LINE_SIZE_MAX + sizeof( uint8_t );

   ck_assert_int_eq( cur_pos_len, sizeof( uint8_t ) );
   ck_assert_int_eq( line_len, REPL_LINE_SIZE_MAX );
   ck_assert_int_eq( *cur_pos, 0 );
   ck_assert_int_eq( g_mheap_top, heap_iter );

   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      line[*cur_pos] = g_chk_str_1[i];
      (*cur_pos)++;
   }

   cur_pos = mget( CHECK_PID, CHECK_MID_CUR_POS1, &cur_pos_len );
   line = mget( CHECK_PID, CHECK_MID_LINE1, &line_len );

   ck_assert_int_eq( cur_pos_len, sizeof( uint8_t ) );
   ck_assert_int_eq( line_len, REPL_LINE_SIZE_MAX );
   ck_assert_int_eq( *cur_pos, CHECK_STR_SZ );
   ck_assert_int_eq( g_mheap_top, heap_iter );

   for( i = 0 ; CHECK_STR_SZ > i ; i++ ) {
      line[*cur_pos] = g_chk_str_2[i];
      (*cur_pos)++;
   }
}
END_TEST
#endif

Suite* mem_suite( void ) {
   Suite* s;
   TCase* tc_core;

   s = suite_create( "mem" );

   /* Core test case */
   tc_core = tcase_create( "Core" );

   tcase_add_test( tc_core, test_mget );
   tcase_add_test( tc_core, test_mshift );
   tcase_add_test( tc_core, test_mget_heap );
   tcase_add_test( tc_core, test_mget_heap_overwrite );
   //tcase_add_test( tc_core, test_mget );
   //tcase_add_test( tc_core, test_metazero );
   tcase_add_test( tc_core, test_mget_pid_match );
   //tcase_add_test( tc_core, test_mset_line );

   suite_add_tcase( s, tc_core );

   return s;
}


