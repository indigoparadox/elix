
#include <check.h>
#include <stdint.h>
#include "../src/mem.h"
#include "../src/console.h"

#include <stdlib.h>

#define CHECK_PID 1

#define CHECK_MID_TEST1 0 
#define CHECK_MID_TEST2 1
#define CHECK_MID_TEST3 2

#define CHECK_MID_LINE1 3
#define CHECK_MID_CUR_POS1 4

#define CHECK_MEM_SHIFT_OFFSET 5

#define CHECK_STR_COUNT 4

char* g_chk_str[4] = {
   "This is a test string.",
   "!@#$%^&*",
   "zxcvbnm",
   "\0"
};

uint8_t g_chk_len[4] = {
   23,
   8,
   7,
   0
};

extern uint8_t g_mheap[MEM_HEAP_SIZE];
extern int g_mheap_top;

uint8_t* g_theap = NULL;

/* When we're setup, we should have:
 * | var | g_chk_str[0] | var | g_chk_str[1] | var | g_chk_str[2]
 */

void setup() {
   struct mvar* var_header = NULL;
   int offset = 0;
   int i = 0;

   minit();

   /* Place the strings on the heap to check. */
   var_header = mget( CHECK_PID, CHECK_MID_TEST1, g_chk_len[0] );
   mcopy( var_header->data, g_chk_str[0], g_chk_len[0] );
   var_header = mget( CHECK_PID, CHECK_MID_TEST2, g_chk_len[1] );
   mcopy( var_header->data, g_chk_str[1], g_chk_len[1] );
   var_header = mget( CHECK_PID, CHECK_MID_TEST3, g_chk_len[2] );
   mcopy( var_header->data, g_chk_str[2], g_chk_len[2] );

   /* Fill up a simulated heap to compare. */
   g_theap = calloc( 1, MEM_HEAP_SIZE );
   do {
      var_header = (struct mvar*)&(g_theap[offset]);
      var_header->pid = CHECK_PID;
      var_header->mid = i;
      var_header->size = g_chk_len[i];
      memcpy( &(var_header->data[0]), g_chk_str[i], g_chk_len[i] );
      offset += sizeof( struct mvar ) + g_chk_len[i];
      i++;
   } while( 0 != g_chk_len[i] );
}

void teardown() {
   free( g_theap );
}

/* Tests: Layout */

START_TEST( test_bytes ) {
   int test_heap = g_theap[_i];
   int mem_heap = g_mheap[_i];
   ck_assert_int_eq( test_heap, mem_heap );
}
END_TEST

START_TEST( test_vars ) {
   struct mvar* var_header = NULL;

   var_header = mget( CHECK_PID, _i, 0 );
   ck_assert_ptr_ne( var_header, NULL );
   ck_assert_int_eq( var_header->mid, _i );
   ck_assert_int_eq( var_header->pid, CHECK_PID );
  // ck_assert_int_eq( var_header->size, g_chk_len[_i] );
   ck_assert_str_eq( var_header->data, g_chk_str[_i] );
}
END_TEST

/* Tests: Shift */

START_TEST( test_mshift ) {
   struct mvar* var_header = NULL;

   mshift( 0, CHECK_MEM_SHIFT_OFFSET );

   var_header = (struct mvar*)&(g_mheap[CHECK_MEM_SHIFT_OFFSET]);  
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, 0 );
   ck_assert_int_eq( var_header->size, g_chk_len[0] );
   
}
END_TEST

/* Tests: Overwrite */

# if 0
START_TEST( test_mget_heap_overwrite ) {
   int i = 0;
   int offset = 0;
   int test_heap = 0;
   int test_sample = 0;
   int correct_heap_top = 0;
   struct mvar* var_header = NULL;

   minit();

   //mprint();


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

#endif
#if 0
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
   TCase* tc_layout;
   TCase* tc_shift;
   //TCase* tc_overwrite;
   //TCase* tc_pid;

   s = suite_create( "mem" );

   /* Test Cases */
   tc_set = tcase_create( "Set" );
   tc_layout = tcase_create( "Layout" );
   tc_shift = tcase_create( "Shift" );
   //tc_overwrite = tcase_create( "Overwrite" );
   //tc_pid = tcase_create( "PID" );

   /* Tests: Set */
   

   /* Tests: Layout */
   tcase_add_checked_fixture( tc_layout, setup, teardown );
   tcase_add_loop_test( tc_layout, test_bytes, 0, MEM_HEAP_SIZE - 1 );
   tcase_add_loop_test( tc_layout, test_vars, 0, CHECK_STR_COUNT - 1 );

   /* Tests: Shift */
   tcase_add_checked_fixture( tc_shift, setup, teardown );
   tcase_add_test( tc_shift, test_mshift );
   tcase_add_loop_test( tc_shift, test_vars, 0, CHECK_STR_COUNT - 1 );

   /* Tests: Overwrite */

   /* Tests: PID */
   //tcase_add_test( tc_core, test_mget_heap );
   //tcase_add_test( tc_core, test_mget_heap_overwrite );
   //tcase_add_test( tc_core, test_mget );
   //tcase_add_test( tc_core, test_metazero );
   //tcase_add_test( tc_core, test_mget_pid_match );
   //tcase_add_test( tc_core, test_mset_line );

   suite_add_tcase( s, tc_layout );
   suite_add_tcase( s, tc_shift );

   return s;
}



