
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

   return pid_iter;
}

int16_t adhd_task_read_short( struct adhd_task* task ) {
   int16_t short_out = 0;
   uint8_t byte_iter = 0;

   mfat_get_dir_entry_data(
      task->file_offset,
      task->proc.ipc,
      (unsigned char*)(&byte_iter), 1,
      task->disk_id, task->part_id );
   short_out = byte_iter;
   short_out <<= 8;
   task->proc.ipc++;
   mfat_get_dir_entry_data(
      task->file_offset,
      task->proc.ipc,
      (unsigned char*)(&byte_iter), 1,
      task->disk_id, task->part_id );
   short_out |= byte_iter;
   
   return short_out;
}

void adhd_task_execute_next( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t instr = 0,
      arg = 0,
      flags = 0;
   VM_SIPC new_ipc = 0;

   assert( 0 <= pid );
   assert( 0 < task->proc.ipc );

   //dprint( "---\nipc: %ld\n", task->ipc );

   instr = adhd_task_read_short( task );

   /* Separate out the flags so we get the instruction index. */
   flags |= instr & VM_MASK_FLAGS;
   instr &= ~VM_MASK_FLAGS;

   /* Sanity checks. */
   assert( instr > 0 );
   assert( instr < VM_OP_MAX );

   arg = adhd_task_read_short( task );

   if( VM_OP_SYSC == instr ) {
      /* SYSC is a special case; call it directly. */
      new_ipc = gc_sysc_cbs[arg]( pid, (uint8_t)(flags & 0xff) );
   } else {
      /* Call VM opcode and let VM handle it. */
      new_ipc = 
         gc_vm_op_cbs[instr]( &(task->proc), (uint8_t)(flags & 0xff), arg );
   }

   if( 0 >= new_ipc || task->sz < new_ipc ) {
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

