
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

   int_test = rand() % 30000;

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &proc, 0, int_test ) );

   ck_assert_int_eq( proc.stack_len, 1 );
   ck_assert_int_eq( int_test, proc.stack[0] );
}
END_TEST

START_TEST( test_vm_op_pop ) {
   int16_t int_test = 0,
      pop_test = 0;

   int_test = rand() % 30000;

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &proc, 0, int_test ) );

   pop_test = vm_op_POP( &proc, 0, 0 );

   ck_assert_int_eq( proc.stack_len, 0 );
   ck_assert_int_eq( int_test, pop_test );
}
END_TEST

START_TEST( test_vm_op_sjump ) {
   int16_t new_ipc = 0,
      old_ipc = 0,
      int_test_1 = 0,
      int_test_2 = 0;

   while( int_test_1 == int_test_2 ) {
      int_test_1 = rand() % 30000;
      int_test_2 = rand() % 30000;
   }
   proc.ipc = rand() % 30000;
   old_ipc = proc.ipc;

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &proc, 0, int_test_1 ) );
   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &proc, 0, int_test_2 ) );

   ck_assert_int_eq( proc.stack_len, 2 );
   ck_assert_int_eq( int_test_1, proc.stack[0] );
   ck_assert_int_eq( int_test_2, proc.stack[1] );

   new_ipc = vm_op_SJUMP( &proc, 0, 0 );

   ck_assert_int_eq( proc.stack_len, 2 );
   ck_assert_int_eq( proc.stack[0], old_ipc + 4 );
   ck_assert_int_eq( proc.stack[1], int_test_1 );
   ck_assert_int_eq( new_ipc, int_test_2 );
}
END_TEST

START_TEST( test_vm_op_sret ) {
   int16_t new_ipc = 0,
      old_ipc = 0,
      int_test_1 = 0,
      int_test_2 = 0;

   while( int_test_1 == int_test_2 ) {
      int_test_1 = rand() % 30000;
      int_test_2 = rand() % 30000;
   }
   proc.ipc = rand() % 30000;
   old_ipc = proc.ipc;

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &proc, 0, int_test_1 ) );
   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &proc, 0, int_test_2 ) );

   vm_op_SJUMP( &proc, 0, 0 );

   ck_assert_int_eq( proc.stack_len, 2 );

   new_ipc = vm_op_SRET( &proc, 0, 0 );

   ck_assert_int_eq( new_ipc, old_ipc + 4 );
   ck_assert_int_eq( proc.stack_len, 1 );
   ck_assert_int_eq( proc.stack[0], int_test_1 );
}
END_TEST

START_TEST( test_vm_op_jseq_eq ) {
   int16_t new_ipc = 0,
      old_ipc = 0,
      int_test_1 = 0,
      int_test_2 = 0;

   while( int_test_1 == int_test_2 ) {
      int_test_1 = rand() % 30000;
      int_test_2 = rand() % 30000;
   }
   proc.ipc = rand() % 30000;
   do {
      old_ipc = proc.ipc;
   } while( proc.ipc == 88 );

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &proc, 0, int_test_1 ) );
   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &proc, 0, int_test_2 ) );
   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &proc, 0, int_test_2 ) );

   ck_assert_int_eq( proc.stack_len, 3 );
   ck_assert_int_eq( proc.stack[0], int_test_1 );
   ck_assert_int_eq( proc.stack[1], int_test_2 );
   ck_assert_int_eq( proc.stack[1], proc.stack[2] );

   new_ipc = vm_op_JSEQ( &proc, 0, 88 );

   ck_assert_int_eq( proc.stack_len, 2 );
   ck_assert_int_eq( proc.stack[0], int_test_1 );
   ck_assert_int_eq( proc.stack[1], int_test_2 );
   ck_assert_int_eq( new_ipc, 88 );
}
END_TEST

START_TEST( test_vm_op_jseq_ne ) {
   int16_t new_ipc = 0,
      old_ipc = 0,
      int_test_1 = 0,
      int_test_2 = 0;

   while( int_test_1 == int_test_2 ) {
      int_test_1 = rand() % 30000;
      int_test_2 = rand() % 30000;
   }
   do {
      proc.ipc = rand() % 30000;
   } while( proc.ipc == 88 );
   old_ipc = proc.ipc;

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &proc, 0, int_test_1 ) );
   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &proc, 0, int_test_2 ) );

   ck_assert_int_eq( proc.stack_len, 2 );
   ck_assert_int_eq( proc.stack[0], int_test_1 );
   ck_assert_int_eq( proc.stack[1], int_test_2 );
   ck_assert_int_ne( proc.stack[1], proc.stack[2] );

   new_ipc = vm_op_JSEQ( &proc, 0, 88 );

   ck_assert_int_eq( proc.stack_len, 1 );
   ck_assert_int_eq( new_ipc, old_ipc + 4 );
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
   tcase_add_test( tc_op, test_vm_op_pop );
   tcase_add_test( tc_op, test_vm_op_sjump );
   tcase_add_test( tc_op, test_vm_op_sret );
   tcase_add_test( tc_op, test_vm_op_jseq_eq );
   tcase_add_test( tc_op, test_vm_op_jseq_ne );

   suite_add_tcase( s, tc_op );

   return s;
}

