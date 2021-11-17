
#include "code16.h"

#define ADHD_C
#include "adhd.h"
#include "mem.h"
#include "vm.h"
#include "sysc.h"

void adhd_start() {
   mzero( g_tasks, sizeof( struct adhd_task ) * ADHD_TASKS_MAX );
}

TASK_PID adhd_task_launch(
   uint8_t disk_id, uint8_t part_id, FILEPTR_T offset
) {
   uint8_t byte_iter = 0;
   TASK_PID pid_iter = 0;
   uint8_t bytes_read = 0,
      cpu_section_found = 0,
      section_instr_found = 0;
   struct adhd_task* task = NULL;

   /* Check for next available PID by using IPC (running tasks will always 
      have IPC > 0!) */
   while( 0 < g_tasks[pid_iter].proc.ipc ) {
      pid_iter++;
   }
   if( ADHD_TASKS_MAX <= pid_iter ) {
      /* Too many tasks already! */
      return RETVAL_TOO_MANY_TASKS;
   }

   /* Zero the whole task, including its stack. */
   mzero( &(g_tasks[pid_iter]), sizeof( struct adhd_task ) );
   task = &(g_tasks[pid_iter]);
   task->disk_id = disk_id;
   task->part_id = part_id;
   task->file_offset = offset;
   task->sz = mfat_get_dir_entry_size( offset, disk_id, part_id );

   /* Move the task IPC past the task data section to the first instruction. */
   do {
      bytes_read = mfat_get_dir_entry_data(
         task->file_offset,
         task->proc.ipc,
         (unsigned char*)(&byte_iter), 1,
         task->disk_id, task->part_id );

      if( 0 == bytes_read ) {
         task->proc.ipc = 0;
         return RETVAL_TASK_INVALID;
      }

      task->proc.ipc += bytes_read;

      if( VM_OP_SECT == byte_iter && !section_instr_found ) {
         section_instr_found = 1;
      } else if( VM_SECTION_CPU == byte_iter && section_instr_found ) {
         cpu_section_found = 1;
      } else {
         section_instr_found = 0;
      }

   } while( !cpu_section_found );

   printf( "starting at offset 0x%02x\n", task->proc.ipc );

   return pid_iter;
}

void adhd_task_read_instr(
   struct adhd_task* task, int16_t* instr, int16_t* arg
) {
   int16_t short_out = 0;
   uint8_t byte_iter = 0;

   /* Read the 16-bit instruction. */
   mfat_get_dir_entry_data(
      task->file_offset,
      task->proc.ipc,
      (unsigned char*)(&byte_iter), 1,
      task->disk_id, task->part_id );
   *instr = byte_iter;
   *instr <<= 8;
   mfat_get_dir_entry_data(
      task->file_offset,
      task->proc.ipc + 1,
      (unsigned char*)(&byte_iter), 1,
      task->disk_id, task->part_id );
   *instr |= byte_iter;
   
   /* Read the 16-bit arg. */
   mfat_get_dir_entry_data(
      task->file_offset,
      task->proc.ipc + 2,
      (unsigned char*)(&byte_iter), 1,
      task->disk_id, task->part_id );
   *arg = byte_iter;
   *arg <<= 8;
   mfat_get_dir_entry_data(
      task->file_offset,
      task->proc.ipc + 3,
      (unsigned char*)(&byte_iter), 1,
      task->disk_id, task->part_id );
   *arg |= byte_iter;
}

#include <stdio.h>

void adhd_task_execute_next( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t instr = 0,
      i = 0,
      arg = 0,
      flags = 0;
   VM_SIPC new_ipc = 0;

   assert( 0 <= pid );
   assert( 0 < task->proc.ipc );

   //dprint( "---\nipc: %ld\n", task->ipc );

   adhd_task_read_instr( task, &instr, &arg );

   /* Separate out the flags so we get the instruction index. */
   flags = instr & VM_MASK_FLAGS;
   instr &= ~VM_MASK_FLAGS;

   /* Sanity checks. */
   if( 0 >= instr ) {
      printf( "execution error: %d\n", instr );
      fflush( stdout );
      assert( instr >= 0 );
   }

   printf(
      "ipc: 0x%02x stack_len: %d instr: 0x%02x flags: 0x%02x arg: 0x%02x\n",
      task->proc.ipc, task->proc.stack_len, instr, flags, arg );
   printf( "--stack: " );
   for( i = 0 ; task->proc.stack_len > i ; i++ ) {
      printf( "0x%02x, ", task->proc.stack[i] );
   }
   printf( "--\n" );

   assert( instr < VM_OP_MAX );
   assert( 0 == flags );

   if( VM_OP_SYSC == instr ) {
      /* SYSC is a special case; call it directly. */
      new_ipc = gc_sysc_cbs[arg]( pid, (uint8_t)(flags & 0xff) );
   } else {
      /* Call VM opcode and let VM handle it. */
      new_ipc = 
         gc_vm_op_cbs[instr]( &(task->proc), (uint8_t)(flags & 0xff), arg );
   }

   if( 0 >= new_ipc || task->sz < new_ipc ) {
      printf( "pid: %d stack_len: %d exiting: %d\n",
         pid, task->proc.stack_len, new_ipc );
      adhd_task_kill( pid );
   } else {
      task->proc.ipc = new_ipc;     
   }
}

void adhd_task_kill( TASK_PID pid ) {
   if( 0 > pid || pid >= ADHD_TASKS_MAX || 0 == g_tasks[pid].proc.ipc ) {
      /* Invalid task index. */
      return;
   }

   /* TODO: Go through memory and remove any allocated blocks for this PID. */
   mfree_all( pid );
   
   g_tasks[pid].proc.ipc = 0;
}

