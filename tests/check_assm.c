
#include <stdlib.h>
#include <check.h>

#include "../src/assm.h"

const char* gc_test_in = 
   ".data:\ntest_str_1: \"Hello\"\ntest_str_2: \"World\"\n"
   ".cpu:\npush start\nsjump\nstart:\npush test_str_2\nsysc puts ; print\n"
   "push 'c'\npush #2\npush #4\nsadd\npush #10\njseq ending\nsret\n\n\n"
   "ending:\npush test_str_1\nsysc puts\n";

unsigned char gc_test_out[] = {
   /*  0 */ 1, /* VM_OP_SECT, */
   /*  1 */ VM_SECTION_DATA,
   /*  2 */ 'H', 'e', 'l', 'l', 'o', '\0',
   /*  8 */ 'W', 'o', 'r', 'l', 'd', '\0',
   /* 14 */ 1, /* VM_OP_SECT, */
   /* 15 */ VM_SECTION_CPU,
   /* 16 */ 0, 0x03, /* VM_OP_PUSH, */
   /* 18 */ 0, 24,   /* Offset of start. */
   /* 20 */ 0, 0x06, /* VM_OP_SJUMP */
   /* 22 */ 0, 0,
   /* start: */
   /* 24 */ 0, 0x03, /* VM_OP_PUSH, */
   /* 26 */ 0, 8,    /* Offset of test_str_2. */
   /* 28 */ 0, 0x02, /* VM_OP_SYSC */
   /* 30 */ 0, 0x02, /* VM_SYSC_PUTS */
   /* 32 */ 0, 0x03, /* VM_OP_PUSH, */
   /* 34 */ 0, 'c',
   /* 36 */ 0, 0x03, /* VM_OP_PUSH, */
   /* 38 */ 0, 2,
   /* 40 */ 0, 0x03, /* VM_OP_PUSH, */
   /* 42 */ 0, 4,
   /* 44 */ 0, 0x05, /* VM_OP_SADD, */
   /* 46 */ 0, 0,
   /* 48 */ 0, 0x03, /* VM_OP_PUSH, */
   /* 50 */ 0, 10,
   /* 52 */ 0, 0x09, /* VM_OP_JSEQ, */
   /* 54 */ 0, 60,   /* Offset of ending. */
   /* 56 */ 0, 0x07, /* VM_OP_SRET */
   /* 58 */ 0, 0,
   /* ending: */
   /* 60 */ 0, 0x03, /* VM_OP_PUSH, */
   /* 62 */ 0, 2,    /* Offset of test_str_1. */
   /* 64 */ 0, 0x02, /* VM_OP_SYSC */
   /* 66 */ 0, 0x02, /* VM_SYSC_PUTS */
};

static struct ASSM_STATE global;

static void setup_assm() {
   int i = 0;

   memset( &global, '\0', sizeof( struct ASSM_STATE ) );
   global.next_alloc_mid = 1;

   for( i = 0 ; strlen( gc_test_in ) > i ; i++ ) {
      process_char( gc_test_in[i], &global );
   }

   assm_resolve_labels( &global );
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
   tcase_add_loop_test( tc_assm, test_assm_encode, 0, 67 );

   suite_add_tcase( s, tc_assm );

   return s;
}

