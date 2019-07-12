
#include <check.h>
#include <stdint.h>
#include "../src/mem.h"

#include <stdlib.h>

#define CHECK_PID 1

#define CHECK_MID_TEST1 1
#define CHECK_MID_TEST2 2
#define CHECK_MID_TEST3 3

#define CHECK_MID_LINE1 4
#define CHECK_MID_CUR_POS1 5

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
   9,
   8,
   0
};

extern uint8_t g_mheap[MEM_HEAP_SIZE];
extern int g_mheap_top;

uint8_t* g_theap = NULL;

/* When we're setup, we should have:
 * | var | g_chk_str[0] | var | g_chk_str[1] | var | g_chk_str[2]
 */

static void setup_mem() {
   struct mvar* var_header = NULL;
   uint8_t* mem_ptr = NULL;
   int offset = 0;
   int i = 0;

#ifdef CHECK_VERBOSE
   printf( "Setting up...\n" );
#endif /* CHECK_VERBOSE */
   g_mheap_top = 0;

   /* Place the strings on the heap to check. */
   for( i = 0 ; 3 > i ; i++ ) {
#ifdef CHECK_VERBOSE
      printf( "Adding \"%s\" to main heap...\n", g_chk_str[i] );
#endif /* CHECK_VERBOSE */
      mem_ptr = (uint8_t*)mget( CHECK_PID, i + 1, g_chk_len[i] );
      ck_assert_ptr_ne( mem_ptr, NULL );
      mcopy( mem_ptr, g_chk_str[i], g_chk_len[i] );
      ck_assert_str_eq( (char*)mem_ptr, g_chk_str[i] );
   }

   /* Fill up a simulated heap to compare. */
   g_theap = calloc( 1, MEM_HEAP_SIZE );
   i = 0;
   do {
#ifdef CHECK_VERBOSE
      printf( "Adding \"%s\" to sim heap...\n", g_chk_str[i] );
#endif /* CHECK_VERBOSE */
      var_header = (struct mvar*)&(g_theap[offset]);
      ck_assert_ptr_ne( var_header, NULL );
      var_header->pid = CHECK_PID;
      var_header->mid = i + 1;
      var_header->size = g_chk_len[i];
      memcpy( &(var_header->data[0]), g_chk_str[i], g_chk_len[i] );
      offset += sizeof( struct mvar ) + g_chk_len[i];
      i++;
   } while( 0 != g_chk_len[i] );
}

static void teardown_mem() {
#ifdef CHECK_VERBOSE
   printf( "Tearing down...\n" );
#endif /* CHECK_VERBOSE */
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
   MEMLEN_T offset = 0;

   offset = mget_pos( CHECK_PID, _i );

   var_header = (struct mvar*)&(g_mheap[offset]);
   ck_assert_ptr_ne( var_header, NULL );
   ck_assert_int_eq( var_header->mid, _i );
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   //ck_assert_int_eq( var_header->size, g_chk_len[_i] );
   //ck_assert_str_eq( var_header->data, g_chk_str[_i] );
}
END_TEST

/* Tests: Shift */

START_TEST( test_mshift ) {
   struct mvar* var_header = NULL;

   mshift( 0, CHECK_MEM_SHIFT_OFFSET );

   /* Shift the first var up by CHECK_MEM_SHIFT_OFFSET and test it. */
   var_header = (struct mvar*)&(g_mheap[CHECK_MEM_SHIFT_OFFSET]);  
   ck_assert_int_eq( var_header->pid, CHECK_PID );
   ck_assert_int_eq( var_header->mid, CHECK_MID_TEST1 );
   ck_assert_int_eq( var_header->size, g_chk_len[0] );
   
}
END_TEST

/* Tests: Overwrite */

#include <stdio.h>
START_TEST( test_mget_overwrite ) {
   uint8_t* mem_ptr = NULL;

   mem_ptr = (uint8_t*)mget( CHECK_PID, _i + 1, g_chk_len[_i] + 5 );
   ck_assert_ptr_ne( mem_ptr, NULL );
   ck_assert_str_eq( (char*)mem_ptr, g_chk_str[_i] );
}
END_TEST

START_TEST( test_mget_pos ) {
   MEMLEN_T pos = 0;

   pos = mget_pos( CHECK_PID, _i );
   switch( _i ) {
      case 1:
         ck_assert_int_eq( pos, 0 );
         break;

      case 2:
         ck_assert_int_eq( pos,
            sizeof( struct mvar ) + g_chk_len[0]  );
         break;

      case 3:
         ck_assert_int_eq( pos,
            (2 * sizeof( struct mvar )) + g_chk_len[0] + g_chk_len[1] );
         break;
   }
}
END_TEST

#if 0
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

/* Tests: Editing */

static void setup_medit() {
   mset( CHECK_PID, 8, g_chk_len[0], &(g_chk_str[0]) );
}

static void teardown_medit() {

}

START_TEST( test_meditprop ) {
   const char* test_str = NULL;
   char test_char = 'q' + _i;

   test_str = mget( CHECK_PID, 8, g_chk_len[0] );
   ck_assert_ptr_ne( test_str, NULL );
   meditprop( CHECK_PID, 8, _i, sizeof( char ), &test_char );
   ck_assert_int_eq( test_str[_i], 'q' + _i );
}
END_TEST

START_TEST( test_mgetprop ) {
   char* test_str = NULL;
   char get_prop_c = '\0';

   test_str = (char*)mget( CHECK_PID, 8, MGET_NO_CREATE );
   ck_assert_ptr_ne( test_str, NULL );

   mgetprop( CHECK_PID, 8, _i, sizeof( char ), &get_prop_c );

   ck_assert_int_eq( test_str[_i], get_prop_c );
   ck_assert_int_ne( get_prop_c, '\0' );
}
END_TEST

Suite* mem_suite( void ) {
   Suite* s;
   TCase* tc_layout;
   TCase* tc_shift;
   //TCase* tc_set;
   TCase* tc_overwrite;
   //TCase* tc_pid;
   TCase* tc_edit;

   s = suite_create( "mem" );

   /* Test Cases */
   //tc_set = tcase_create( "Set" );
   tc_layout = tcase_create( "Layout" );
   tc_shift = tcase_create( "Shift" );
   tc_overwrite = tcase_create( "Overwrite" );
   //tc_pid = tcase_create( "PID" );
   tc_edit = tcase_create( "Edit" );

   /* Tests: Set */
   

   /* Tests: Layout */
   tcase_add_checked_fixture( tc_layout, setup_mem, teardown_mem );
   tcase_add_loop_test( tc_layout, test_bytes, 0, MEM_HEAP_SIZE - 1 );
   tcase_add_loop_test( tc_layout, test_vars, 1, CHECK_STR_COUNT );

   /* Tests: Shift */
   tcase_add_checked_fixture( tc_shift, setup_mem, teardown_mem );
   tcase_add_test( tc_shift, test_mshift );
   tcase_add_loop_test( tc_shift, test_vars, 1, CHECK_STR_COUNT );

   /* Tests: Overwrite */
   tcase_add_checked_fixture( tc_overwrite, setup_mem, teardown_mem );
   tcase_add_loop_test( tc_overwrite, test_mget_overwrite, 0, 3 );
   tcase_add_loop_test( tc_overwrite, test_mget_pos, 1, 4 );

   /* Tests: PID */
   //tcase_add_test( tc_core, test_mget_heap );
   //tcase_add_test( tc_core, test_mget );
   //tcase_add_test( tc_core, test_metazero );
   //tcase_add_test( tc_core, test_mget_pid_match );
   //tcase_add_test( tc_core, test_mset_line );

   /* Tests: Edit */
   tcase_add_checked_fixture( tc_overwrite, setup_medit, teardown_medit );
   tcase_add_loop_test( tc_edit, test_meditprop, 1, 4 );
   tcase_add_loop_test( tc_edit, test_mgetprop, 1, 4 );

   suite_add_tcase( s, tc_overwrite );
   suite_add_tcase( s, tc_layout );
   suite_add_tcase( s, tc_shift );
   suite_add_tcase( s, tc_edit );

   return s;
}



