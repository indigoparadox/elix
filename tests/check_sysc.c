
#include <check.h>
#include <time.h>
#include <stdlib.h>

#define ADHD_C
#include "../src/vm.h"
#include "../src/sysc.h"
#include "../src/adhd.h"
#include "../src/mem.h"

#define TEST_PID 0
#define TEST_MID 1
#define TEST_MEM_SZ 10

struct ADHD_TASK* task = &(g_tasks[TEST_PID]);

static void setup_vm_mem() {
   memset( g_tasks, '\0', sizeof( struct ADHD_TASK ) * 10 );
   minit();
}

static void teardown_vm_mem() {
}

START_TEST( test_sysc_malloc ) {
   void* ptr = NULL;

   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &(task->proc), 0, TEST_MEM_SZ ) );
   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &(task->proc), 0, TEST_MID ) );

   ck_assert_int_eq( task->proc.ipc + 4, sysc_MALLOC( TEST_PID, 0 ) );

   ptr = mget( TEST_PID, TEST_MID, 0 );

   ck_assert_ptr_ne( NULL, ptr );
}
END_TEST

START_TEST( test_sysc_mpush ) {
   uint8_t* ptr = NULL;

   /* Allocate memory. */
   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &(task->proc), 0, TEST_MEM_SZ ) );
   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &(task->proc), 0, TEST_MID ) );

   ck_assert_int_eq( task->proc.ipc + 4, sysc_MALLOC( TEST_PID, 0 ) );

   /* Setup test values. */
   ptr = mget( TEST_PID, TEST_MID, 0 );

   ck_assert_ptr_ne( NULL, ptr );

   ptr[0] = 127;
   ptr[1] = 127;

   /* Try to push test values from memory to stack. */
   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &(task->proc), 0, 0 ) ); /* Offset */
   ck_assert_int_ne(
      VM_ERROR_STACK, vm_op_SPUSH( &(task->proc), 0, TEST_MID ) );

   ck_assert_int_eq( task->proc.ipc + 4, sysc_MPUSH( TEST_PID, 0 ) );

   /* Check results. */
   ck_assert_int_eq( task->proc.stack[0], 32639 );
}
END_TEST

Suite* sysc_suite( void ) {
   Suite* s;
   TCase* tc_mem;

   s = suite_create( "sysc" );

   /* Core test case */
   tc_mem = tcase_create( "Memory Syscalls" );

   tcase_add_checked_fixture( tc_mem, setup_vm_mem, teardown_vm_mem );
   tcase_add_test( tc_mem, test_sysc_malloc );
   tcase_add_test( tc_mem, test_sysc_mpush );

   suite_add_tcase( s, tc_mem );

   return s;
}

