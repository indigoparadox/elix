
#include "code16.h"

#include "vm.h"
#include "console.h"
#include "adhd.h"

static uint8_t g_double_instr = 0;

static uint8_t vm_stack_pop( struct adhd_task* task ) {
   assert( 0 < task->stack_len );
   task->stack_len--;
   return task->stack[task->stack_len];
}

static uint16_t vm_stack_dpop( struct adhd_task* task ) {
   uint16_t dout = 0;
   assert( 1 < task->stack_len );
   task->stack_len--;
   dout |= (task->stack[task->stack_len]) & 0x00ff;
   task->stack_len--;
   dout |= (task->stack[task->stack_len] << 8) & 0xff00;
   return dout;
}

static void vm_stack_push( struct adhd_task* task, uint8_t data ) {
   assert( ADHD_STACK_MAX > task->stack_len + 1 );
   task->stack[task->stack_len] = data;
   task->stack_len++;
}

static void vm_stack_dpush( struct adhd_task* task, uint16_t data ) {
   assert( ADHD_STACK_MAX > task->stack_len + 2 );
   task->stack[task->stack_len] = (uint8_t)((data >> 8) & 0x00ff);
   task->stack_len++;
   task->stack[task->stack_len] = (uint8_t)(data & 0x00ff);
   task->stack_len++;
}

union vm_data_type {
   uint8_t byte;
   IPC_PTR ipc;
};

static void vm_sysc_droot( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint16_t offset = 0;
   uint8_t disk_id = 0,
      part_id = 0;

   part_id = vm_stack_pop( task );
   disk_id = vm_stack_pop( task );

   offset = mfat_get_root_dir_offset( disk_id, part_id );

   vm_stack_dpush( task, offset );
}

static void vm_sysc_dfirst( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint16_t offset = 0;
   uint8_t disk_id = 0,
      part_id = 0;

   part_id = vm_stack_pop( task );
   disk_id = vm_stack_pop( task );
   offset = vm_stack_dpop( task );

   offset = mfat_get_dir_entry_first_offset( offset, disk_id, part_id );

   vm_stack_dpush( task, offset );
}

static void vm_instr_sysc( TASK_PID pid, uint8_t call_id ) {
   struct adhd_task* task = &(g_tasks[pid]);
   union vm_data_type data;
   unsigned char cbuf = 0;
   uint8_t bytes_read = 0;

   assert( 0 <= pid );
   assert( 0 <= task->ipc );

   switch( call_id ) {
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

   case VM_SYSC_DROOT:
      vm_sysc_droot( pid );
      break;
      
   case VM_SYSC_DFIRST:
      vm_sysc_dfirst( pid );
      break;
      
   case VM_SYSC_DNEXT:
      vm_sysc_dnext( pid );
      break;
      
   case VM_SYSC_DNAME:
      vm_sysc_dname( pid );
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

   case VM_INSTR_JSGE:
      comp2 = vm_stack_pop( task );
      comp1 = vm_stack_pop( task );
      if( comp1 >= comp2 ) {
         retval = data;
      }
      break;
   }

   vm_stack_push( task, comp1 );

   return retval;
}

static ssize_t vm_instr_mem( TASK_PID pid, uint8_t instr, uint8_t data ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t* addr_tmp = NULL;
   uint8_t offset = 0;

   switch( instr ) {
   case VM_INSTR_MALLOC:
      addr_tmp = mget( pid, data, vm_stack_pop( task ) );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      break;

   case VM_INSTR_MPOP:
      addr_tmp = mget( pid, data, 0 );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      *addr_tmp = vm_stack_pop( task );
      break;

   case VM_INSTR_MPOPC:
      addr_tmp = mget( pid, data, 0 );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      *addr_tmp = vm_stack_pop( task );
      vm_stack_push( task, *addr_tmp );
      break;

   case VM_INSTR_MPOPCO:
      addr_tmp = mget( pid, data, 0 );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      offset = vm_stack_pop( task );
      *(addr_tmp + offset) = vm_stack_pop( task );
      vm_stack_push( task, *(addr_tmp + offset) );
      break;

   case VM_INSTR_MPUSHC:
      addr_tmp = mget( pid, data, 0 );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      vm_stack_push( task, *addr_tmp );
      break;

   case VM_INSTR_MPUSHCO:
      addr_tmp = mget( pid, data, 0 );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      offset = vm_stack_pop( task );
      vm_stack_push( task, *(addr_tmp + offset) );
      break;
   }

   return task->ipc + 1;
}

ssize_t vm_instr_execute( TASK_PID pid, uint16_t instr_full ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t instr = instr_full >> 8;
   uint8_t data = instr_full & 0xff;

   assert( 0 <= pid );
   assert( 0 <= task->ipc );

   //printf( "ipc: %d, instr: %d (0x%x), data: %d (0x%x), stack: %d\n", task->ipc, instr, instr, data, data, task->stack_len );

   //assert( 0 != instr );

   switch( g_double_instr ) {
   case VM_INSTR_DPUSH:
      vm_stack_dpush( task, instr_full );
      return task->ipc + 1; /* Done processing. */

   default:
      break; /* Ignore. */
   }

   switch( instr ) {
   case VM_INSTR_PUSH:
      vm_stack_push( task, data );
      break;

   case VM_INSTR_DPUSH:
      /* Push will happen on next execute with full number. */
      g_double_instr = VM_INSTR_DPUSH;
      break;

   case VM_INSTR_SPOP:
      vm_stack_pop( task );
      break;

   case VM_INSTR_SDPOP:
      vm_stack_pop( task );
      break;

   case VM_INSTR_SADD:
      data = vm_stack_pop( task ) + vm_stack_pop( task );
      vm_stack_push( task, data );
      break;

   case VM_INSTR_SYSC:
      vm_instr_sysc( pid, data );
      break;
   
   case VM_INSTR_EXIT:
      return -1;

   case VM_INSTR_GOTO:
      return data;

   case VM_INSTR_JSNZ:
   case VM_INSTR_JSEQ:
   case VM_INSTR_JSNE:
   case VM_INSTR_JSGE:
      return vm_instr_branch( pid, instr, data );

   case VM_INSTR_MALLOC:
   case VM_INSTR_MPOP:
   case VM_INSTR_MPOPC:
   case VM_INSTR_MPOPCO:
   case VM_INSTR_MPUSHC:
   case VM_INSTR_MPUSHCO:
      return vm_instr_mem( pid, instr, data );
   }

   return task->ipc + 1;
}

