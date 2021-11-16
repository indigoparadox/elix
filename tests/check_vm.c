
#include <check.h>
#include <time.h>
#include <stdlib.h>

#include "../src/vm.h"

struct VM_PROC proc;

static void setup_vm() {
   srand( time( NULL ) );
   memset( &proc, '\0', sizeof( struct VM_PROC ) );
}

static void teardown_vm() {
}

START_TEST( test_vm_op_push ) {
   uint8_t int_test = 0;

   int_test = rand() % 255;

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_PUSH( &proc, 0, int_test ) );

   ck_assert_int_eq( proc.stack_len, 1 );
   ck_assert_int_eq( int_test, proc.stack[0] );
}
END_TEST

START_TEST( test_vm_op_pushd ) {
   int16_t int_test = 0;

   int_test = rand() % 30000;

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_PUSH( &proc, VM_FLAG_DBL, int_test ) );

   ck_assert_int_eq( proc.stack_len, 2 );
   ck_assert_int_eq( int_test & 0xff, proc.stack[1] );
   ck_assert_int_eq( (int_test & 0xff00) >> 8, proc.stack[0] );
}
END_TEST

START_TEST( test_vm_op_pop ) {
   uint8_t int_test = 0,
      pop_test = 0;

   int_test = rand() % 255;

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_PUSH( &proc, 0, int_test ) );

   pop_test = vm_op_POP( &proc, 0, 0 );

   ck_assert_int_eq( proc.stack_len, 0 );
   ck_assert_int_eq( int_test, pop_test );
}
END_TEST

START_TEST( test_vm_op_popd ) {
   int16_t int_test = 0,
      pop_test = 0;

   int_test = rand() % 30000;

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_PUSH( &proc, VM_FLAG_DBL, int_test ) );

   pop_test = vm_op_POP( &proc, VM_FLAG_DBL, 0 );

   ck_assert_int_eq( proc.stack_len, 0 );
   ck_assert_int_eq( int_test, pop_test );
}
END_TEST

START_TEST( test_vm_op_saddd ) {
   int16_t int_test_1 = 0,
      int_test_2 = 0,
      sum_pop = 0,
      sum_test = 0;

   int_test_1 = rand() % 15000;
   int_test_2 = rand() % 15000;
   sum_test = int_test_1 + int_test_2;

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_PUSH( &proc, VM_FLAG_DBL, int_test_1 ) );
   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_PUSH( &proc, VM_FLAG_DBL, int_test_2 ) );

   ck_assert_int_eq( proc.stack_len, 4 );

   vm_op_SADD( &proc, VM_FLAG_DBL, 0 );

   sum_pop = vm_op_POP( &proc, VM_FLAG_DBL, 0 );

   /* ck_assert_int_eq( proc.stack_len, 0 ); */
   ck_assert_int_eq( sum_pop, sum_test );
}
END_TEST

Suite* vm_suite( void ) {
   Suite* s;
   TCase* tc_op;

   s = suite_create( "vm" );

   /* Core test case */
   tc_op = tcase_create( "Opcodes" );

   tcase_add_checked_fixture( tc_op, setup_vm, teardown_vm );
   tcase_add_test( tc_op, test_vm_op_push );
   tcase_add_test( tc_op, test_vm_op_pushd );
   tcase_add_test( tc_op, test_vm_op_pop );
   tcase_add_test( tc_op, test_vm_op_popd );
   tcase_add_test( tc_op, test_vm_op_saddd );

   suite_add_tcase( s, tc_op );

   return s;
}

