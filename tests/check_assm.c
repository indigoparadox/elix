
#include <check.h>

#include "../src/assm.h"

const char* gc_test_in = 
   ".data:\ntest_str_1: \"Hello\"\ntest_str_2: \"World\"\n"
   ".cpu:\npushd test_str_2\nsyscall puts\n"
   "pushd #2\npushd #4\nsaddd\n";

unsigned char gc_test_out[] = {
   /*  0 */ 1, /* VM_OP_SECT, */
   /*  1 */ VM_SECTION_DATA,
   /*  2 */ 'H', 'e', 'l', 'l', 'o', '\0',
   /*  8 */ 'W', 'o', 'r', 'l', 'd', '\0',
   /* 14 */ 1, /* VM_OP_SECT, */
   /* 15 */ VM_SECTION_CPU,
   /* 16 */ 0, 0x83, /* VM_OP_PUSH | VM_FLAG_DBL, */
   /* 18 */ 0, 8, /* Offset of test_str. */
   /* 20 */ 0, 0x02, /* VM_OP_SYSC */
   /* 22 */ 0, 0x02, /* VM_SYSC_PUTS */
   /* 24 */ 0, 0x83, /* VM_OP_PUSH | VM_FLAG_DBL, */
   /* 26 */ 0, 2,
   /* 28 */ 0, 0x83, /* VM_OP_PUSH | VM_FLAG_DBL, */
   /* 30 */ 0, 4,
   /* 32 */ 0, 0x85, /* VM_OP_PUSH | VM_FLAG_DBL, */
};

static struct ASSM_STATE global;

static void setup_assm() {
   int i = 0;

   memset( &global, '\0', sizeof( struct ASSM_STATE ) );
   global.next_alloc_mid = 1;

   for( i = 0 ; strlen( gc_test_in ) > i ; i++ ) {
      process_char( gc_test_in[i], &global );
   }
}

static void teardown_assm() {
   if( NULL != global.out_buffer ) {
      free( global.out_buffer );
      global.out_buffer = NULL;
   }
}

START_TEST( test_assm_encode ) {
   ck_assert_uint_eq( global.out_buffer[_i], gc_test_out[_i] );
}
END_TEST

Suite* assm_suite( void ) {
   Suite* s;
   TCase* tc_assm;

   s = suite_create( "assm" );

   /* Core test case */
   tc_assm = tcase_create( "Assemble" );

   tcase_add_checked_fixture( tc_assm, setup_assm, teardown_assm );
   tcase_add_loop_test( tc_assm, test_assm_encode, 0, 33 );

   suite_add_tcase( s, tc_assm );

   return s;
}

