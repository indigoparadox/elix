
#include <check.h>
#include <stdint.h>
#include "../src/mem.h"
#include "../src/ring.h"

#define CHECK_PID 1

#define CHECK_MID_TEST1 1
#define CHECK_MID_TEST2 2
#define CHECK_MID_TEST3 3

#define CHECK_RB_SZ 10

static void setup_ring() {
   minit();

/*
const struct ring_buffer* ring_buffer( uint8_t pid, MEM_ID mid, MEMLEN_T sz );
void ring_buffer_push( uint8_t pid, MEM_ID mid, char val );
char ring_buffer_pop( uint8_t pid, MEM_ID mid );
void ring_buffer_wait( struct ring_buffer* buffer );
*/
}

static void teardown_ring() {
}

START_TEST( test_ring_alloc ) {
   const struct ring_buffer* rb = NULL;

   rb = ring_buffer( CHECK_PID, CHECK_MID_TEST1, CHECK_RB_SZ );

   ck_assert_int_eq( rb->sz, CHECK_RB_SZ );
}
END_TEST

#include <stdio.h>

static void setup_ring_wrap() {
   minit();
}

static void teardown_ring_wrap() {

}

START_TEST( test_ring_push ) {
   const struct ring_buffer* rb = NULL;
   MEMLEN_T idx = (_i % CHECK_RB_SZ) - 1;
   MEMLEN_T next_end = (_i % (CHECK_RB_SZ + 1));
   /* int i = 0; */

   rb = ring_buffer( CHECK_PID, CHECK_MID_TEST2, CHECK_RB_SZ );

   ring_buffer_push( CHECK_PID, CHECK_MID_TEST2, _i );

   /* for( i = 0 ; CHECK_RB_SZ > i ; i++ ) {
      printf( "%d - %c\n", i, rb->buffer[i] );
   } */

   ck_assert_int_le( rb->end, CHECK_RB_SZ );
   ck_assert_int_eq( rb->end, next_end );
   ck_assert_int_eq( rb->buffer[idx], _i );
}
END_TEST

START_TEST( test_ring_pop ) {
   const struct ring_buffer* rb = NULL;
   MEMLEN_T idx = (_i % CHECK_RB_SZ) - 1;
   /* int i = 0; */
   char c = '\0';

   rb = ring_buffer( CHECK_PID, CHECK_MID_TEST2, CHECK_RB_SZ );

   ring_buffer_push( CHECK_PID, CHECK_MID_TEST2, _i );

   c = ring_buffer_pop( CHECK_PID, CHECK_MID_TEST2 );

   /* for( i = 0 ; CHECK_RB_SZ > i ; i++ ) {
      printf( "%d - %c\n", i, rb->buffer[i] );
   } */

   ck_assert_int_eq( c, _i );
   ck_assert_int_lt( rb->start, CHECK_RB_SZ );
   ck_assert_int_eq( rb->buffer[idx], _i );
}
END_TEST

Suite* ring_suite( void ) {
   Suite* s;
   TCase* tc_alloc;
   TCase* tc_edit;

   s = suite_create( "ring" );

   /* Test Cases */
   tc_alloc = tcase_create( "Alloc" );
   tc_edit = tcase_create( "Edit" );
   
   tcase_add_checked_fixture( tc_alloc, setup_ring, teardown_ring );
   tcase_add_test( tc_alloc, test_ring_alloc );

   tcase_add_unchecked_fixture( tc_edit, setup_ring_wrap, teardown_ring_wrap );
   tcase_add_loop_test( tc_edit, test_ring_push, 1, 20 );
   tcase_add_loop_test( tc_edit, test_ring_pop, 0, CHECK_RB_SZ - 1 );

   suite_add_tcase( s, tc_alloc );
   suite_add_tcase( s, tc_edit );

   return s;
}

