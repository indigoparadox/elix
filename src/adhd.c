
#include "code16.h"

#define ADHD_C
#include "adhd.h"
#include "mem.h"

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
   while( 0 < g_tasks[pid_iter].state.ipc ) {
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
         task->state.ipc,
         (unsigned char*)(&byte_iter), 1,
         task->disk_id, task->part_id );

      if( 0 == bytes_read ) {
         task->state.ipc = 0;
         return RETVAL_TASK_INVALID;
      }

      task->state.ipc += bytes_read;

      if( VM_INSTR_SECT == byte_iter && !section_instr_found ) {
         section_instr_found = 1;
      } else if( VM_SECTION_CPU == byte_iter && section_instr_found ) {
         cpu_section_found = 1;
      } else {
         section_instr_found = 0;
      }

   } while( !cpu_section_found );

   return pid_iter;
}

void adhd_task_execute_next( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint16_t instr = 0;
   ssize_t new_ipc = 0;
   uint8_t byte_iter = 0;

   assert( 0 <= pid );
   assert( 0 < task->state.ipc );

   //dprint( "---\nipc: %ld\n", task->ipc );

   mfat_get_dir_entry_data(
      task->file_offset,
      task->state.ipc,
      (unsigned char*)(&byte_iter), 1,
      task->disk_id, task->part_id );
   //dprint( "instr upper: %04x\n", instr );
   instr = byte_iter;
   instr <<= 8;
   task->state.ipc++;
   mfat_get_dir_entry_data(
      task->file_offset,
      task->state.ipc,
      (unsigned char*)(&byte_iter), 1,
      task->disk_id, task->part_id );
   instr |= byte_iter;
   //dprint( "instr: %04x\n", instr );

   new_ipc = vm_instr_execute( pid, &(task->state), instr );
   if( 0 >= new_ipc || task->sz < new_ipc ) {
      adhd_task_kill( pid );
   } else {
      task->state.ipc = new_ipc;     
   }
}

void adhd_task_kill( TASK_PID pid ) {
   if( 0 > pid || pid >= ADHD_TASKS_MAX || 0 == g_tasks[pid].state.ipc ) {
      /* Invalid task index. */
      return;
   }

   /* TODO: Go through memory and remove any allocated blocks for this PID. */
   mfree_all( pid );
   
   g_tasks[pid].state.ipc = 0;
}

