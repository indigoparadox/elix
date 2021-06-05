
#include "code16.h"

#include "vm.h"
#include "console.h"
#include "adhd.h"

static uint8_t vm_stack_pop( struct adhd_task* task ) {
   assert( 0 < task->stack_len );

   task->stack_len--;

   return task->stack[task->stack_len];
}

static void vm_stack_push( struct adhd_task* task, uint8_t data ) {

   assert( ADHD_STACK_MAX > task->stack_len + 1 );

   task->stack[task->stack_len] = data;

   task->stack_len++;
}

static void vm_instr_sysc( struct adhd_task* task, uint8_t call_id ) {
   uint8_t data = 0;

   switch( call_id ) {
   case VM_SYSC_PUTC:
      data = vm_stack_pop( task );
      tputc( data );
      break;
   }
}

ssize_t vm_instr_execute( int8_t pid, uint16_t instr_full ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t instr = instr_full >> 8;
   uint8_t data = instr_full & 0xff;

   assert( 0 <= pid );
   assert( 0 <= task->ipc );

   switch( instr ) {
   case VM_INSTR_PUSH:
      vm_stack_push( task, data );
      break;

   case VM_INSTR_SYSC:
      vm_instr_sysc( task, data );
      break;
   
   case VM_INSTR_EXIT:
      return -1;

   case VM_INSTR_GOTO:
      return data;
   }

   return task->ipc + 1;
}

