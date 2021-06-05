
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

union vm_data_type {
   uint8_t byte;
   IPC_PTR ipc;
};

static void vm_instr_sysc( TASK_PID pid, uint8_t call_id ) {
   struct adhd_task* task = &(g_tasks[pid]);
   union vm_data_type data;
   unsigned char cbuf = 0;
   uint8_t bytes_read = 0;

   assert( 0 <= pid );
   assert( 0 <= task->ipc );

   switch( call_id ) {
   case VM_SYSC_ALLOC:
      mget( pid, vm_stack_pop( task ), vm_stack_pop( task ) );
      break;

   case VM_SYSC_PUTC:
      data.byte = vm_stack_pop( task );
      tputc( data.byte );
      break;

   case VM_SYSC_GETC:
      cbuf = tgetc();
      vm_stack_push( task, cbuf );
      break;

   case VM_SYSC_PRINTF:
      /* TODO: Use actual printf w/ format strings. */
      data.ipc = vm_stack_pop( task );
      bytes_read = mfat_get_dir_entry_data(
         task->file_offset,
         data.ipc,
         &cbuf, 1,
         task->disk_id, task->part_id );
      assert( 0 < bytes_read ); /* TODO: Crash program. */
      while( 0 < cbuf ) {
         tputc( cbuf );
         data.ipc += 1;
         bytes_read = mfat_get_dir_entry_data(
            task->file_offset,
            data.ipc,
            &cbuf, 1,
            task->disk_id, task->part_id );
         assert( 0 < bytes_read ); /* TODO: Crash program. */
      }
      break;
   }

   /* TODO: Q&D defeat warning. */
   if( bytes_read != 0 ) {
      bytes_read = 1;
   }
}

static ssize_t vm_instr_branch( TASK_PID pid, uint8_t instr, uint8_t data ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t comp1 = 0,
      comp2 = 0;
   ssize_t retval = task->ipc + 1;
   
   switch( instr ) {
   case VM_INSTR_JSNZ:
      comp1 = vm_stack_pop( task );
      if( 0 != comp1 ) {
         retval = data;
      }
      break;

   case VM_INSTR_JSEQ:
      comp2 = vm_stack_pop( task );
      comp1 = vm_stack_pop( task );
      if( comp1 == comp2 ) {
         retval = data;
      }
      break;

   case VM_INSTR_JSNE:
      comp2 = vm_stack_pop( task );
      comp1 = vm_stack_pop( task );
      if( comp1 != comp2 ) {
         retval = data;
      }
      break;
   }

   vm_stack_push( task, comp1 );

   return retval;
}

ssize_t vm_instr_execute( TASK_PID pid, uint16_t instr_full ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t instr = instr_full >> 8;
   uint8_t data = instr_full & 0xff;

   assert( 0 <= pid );
   assert( 0 <= task->ipc );

   //printf( "ipc: %d, instr: %d (0x%x), data: %d (0x%x)\n",
   //   task->ipc, instr, instr, data, data );

   //assert( 0 != instr );

   switch( instr ) {
   case VM_INSTR_PUSH:
      vm_stack_push( task, data );
      break;

   case VM_INSTR_POP:
      vm_stack_pop( task );
      break;

   case VM_INSTR_SYSC:
      vm_instr_sysc( pid, data );
      break;
   
   case VM_INSTR_EXIT:
      return -1;

   case VM_INSTR_GOTO:
      return data;

   case VM_INSTR_JSNZ:
      return vm_instr_branch( pid, instr, data );

   case VM_INSTR_JSEQ:
      return vm_instr_branch( pid, instr, data );

   case VM_INSTR_JSNE:
      return vm_instr_branch( pid, instr, data );
   }

   return task->ipc + 1;
}

