
#include "code16.h"

#include "uprintf.h"
#define ADHD_C
#include "adhd.h"
#include "mem.h"
#include "vm.h"
#include "sysc.h"

void adhd_init() {
   mzero( g_tasks, sizeof( struct ADHD_TASK ) * ADHD_TASKS_MAX );
   mzero( g_files, sizeof( struct ADHD_FILE ) * ADHD_FILES_MAX );
}

int8_t adhd_file_open(
   uint8_t disk_id, uint8_t part_id, uint32_t offset, uint8_t flags
) {
   int8_t i = 0;

   for( i = 0 ; ADHD_FILES_MAX > i ; i++ ) {
      if(
         ADHD_FILE_FLAG_OPEN == (g_files[i].flags & ADHD_FILE_FLAG_OPEN) &&
         g_files[i].offset == offset &&
         g_files[i].disk_id == disk_id &&
         g_files[i].part_id == part_id
      ) {
         /* File is already open. */
         return i;
      }
   }
      
   /* File was not open, so try to open it. */
   for( i = 0 ; ADHD_FILES_MAX > i ; i++ ) {
      if(
         ADHD_FILE_FLAG_OPEN != (g_files[i].flags & ADHD_FILE_FLAG_OPEN)
      ) {
         /* Create new file entry. */
         elix_dprintf( 1, "opening new file: %d @ %d", i, offset );
         g_files[i].offset = offset;
         g_files[i].disk_id = disk_id;
         g_files[i].part_id = part_id;
         g_files[i].flags = ADHD_FILE_FLAG_OPEN | flags;
         g_files[i].sz = mfat_get_dir_entry_size( offset, disk_id, part_id );
         return i;
      }
   }

   return ADHD_ERROR_NO_FILES_AVAIL;
}

void adhd_file_close( uint8_t file_id ) {
   int8_t i = 0;

   assert(
      ADHD_FILE_FLAG_OPEN == (g_files[file_id].flags & ADHD_FILE_FLAG_OPEN) );

   elix_dprintf(
      1, "closing file: %d @ %d", file_id, g_files[file_id].offset );

   mzero( &(g_files[file_id]), sizeof( struct ADHD_FILE ) );
}

int8_t adhd_file_open_path(
   uint8_t disk_id, uint8_t part_id, const char* path, uint8_t flags
) {
   uint32_t root_offset = 0,
      file_offset = 0;

   root_offset = mfat_get_root_dir_offset( 0, 0 );
   file_offset = mfat_get_dir_entry_offset(
      path, MFAT_FILENAME_LEN, root_offset, 0, 0 );
 
   if( 0 == file_offset ) {
      return ADHD_ERROR_FILE_NOT_FOUND;
   }

   return adhd_file_open( disk_id, part_id, file_offset, flags );
}

TASK_PID adhd_task_launch( int8_t file_id ) {
   uint8_t byte_iter = 0;
   TASK_PID pid_iter = 0;
   uint8_t bytes_read = 0,
      cpu_section_found = 0,
      section_instr_found = 0;
   struct ADHD_TASK* task = NULL;

   assert( 0 <= file_id );
   assert(
      ADHD_FILE_FLAG_EXEC == (g_files[file_id].flags & ADHD_FILE_FLAG_EXEC) );

   /* Check for next available PID by using IPC (running tasks will always 
      have IPC > 0!) */
   while( 0 < g_tasks[pid_iter].proc.ipc ) {
      pid_iter++;
   }
   if( ADHD_TASKS_MAX <= pid_iter ) {
      /* Too many tasks already! */
      return ADHD_ERROR_NO_TASKS_AVAIL;
   }

   /* Zero the whole task, including its stack. */
   mzero( &(g_tasks[pid_iter]), sizeof( struct ADHD_TASK ) );
   task = &(g_tasks[pid_iter]);
   task->file_id = file_id;

   /* Move the task IPC past the task data section to the first instruction. */
   do {
      bytes_read = mfat_get_dir_entry_data(
         g_files[task->file_id].offset,
         task->proc.ipc,
         (unsigned char*)(&byte_iter), 1,
         g_files[task->file_id].disk_id, g_files[task->file_id].part_id );

      if( 0 == bytes_read ) {
         task->proc.ipc = 0;
         return ADHD_ERROR_TASK_NOT_FOUND;
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

   elix_dprintf( 1, "starting at offset 0x%02x", task->proc.ipc );

   return pid_iter;
}

static void adhd_task_read_instr(
   struct ADHD_TASK* task, int16_t* instr, int16_t* arg
) {
   int16_t short_out = 0;
   uint8_t byte_iter = 0;

   /* Read the 16-bit instruction. */
   mfat_get_dir_entry_data(
      g_files[task->file_id].offset,
      task->proc.ipc,
      (unsigned char*)(&byte_iter), 1,
      g_files[task->file_id].disk_id, g_files[task->file_id].part_id );
   *instr = byte_iter;
   *instr <<= 8;
   mfat_get_dir_entry_data(
      g_files[task->file_id].offset,
      task->proc.ipc + 1,
      (unsigned char*)(&byte_iter), 1,
      g_files[task->file_id].disk_id, g_files[task->file_id].part_id );
   *instr |= byte_iter;
   
   /* Read the 16-bit arg. */
   mfat_get_dir_entry_data(
      g_files[task->file_id].offset,
      task->proc.ipc + 2,
      (unsigned char*)(&byte_iter), 1,
      g_files[task->file_id].disk_id, g_files[task->file_id].part_id );
   *arg = byte_iter;
   *arg <<= 8;
   mfat_get_dir_entry_data(
      g_files[task->file_id].offset,
      task->proc.ipc + 3,
      (unsigned char*)(&byte_iter), 1,
      g_files[task->file_id].disk_id, g_files[task->file_id].part_id );
   *arg |= byte_iter;
}

#include <stdio.h>

void adhd_task_execute_next( TASK_PID pid ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t instr = 0,
      i = 0,
      arg = 0,
      old_ipc = task->proc.ipc,
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
      elix_eprintf( "execution error: %d", instr );
      assert( instr >= 0 );
   }

   if( VM_OP_MAX <= instr ) {
      elix_eprintf( "invalid instruction: 0x%02x offset: 0x%02x",
         instr, task->proc.ipc );
      adhd_task_kill( pid );
      return;
   }
   assert( 0 == flags );

   if( VM_OP_SYSC == instr ) {
      /* SYSC is a special case; call it directly. */
      new_ipc = gc_sysc_cbs[arg]( pid, (uint8_t)(flags & 0xff) );
   } else {
      /* Call VM opcode and let VM handle it. */
      new_ipc = 
         gc_vm_op_cbs[instr]( &(task->proc), (uint8_t)(flags & 0xff), arg );
   }

   elix_dprintf( 0, "ipc: 0x%02x stack_len: %d instr: %s flags: 0x%02x",
      old_ipc, task->proc.stack_len, gc_vm_op_tokens[instr], flags );
   if( VM_OP_SYSC == instr ) {
      elix_dprintf( 0, "arg: %s ret: %d", gc_sysc_tokens[arg], new_ipc );
   } else {
      elix_dprintf( 0, "arg: 0x%02x ret: %d", arg, new_ipc );
   }
   elix_dprintf( 0, "--stack:" );
   for( i = 0 ; task->proc.stack_len > i ; i++ ) {
      elix_dprintf( 0, "   0x%02x,", task->proc.stack[i] );
   }

   if( 0 >= new_ipc || g_files[task->file_id].sz < new_ipc ) {
      elix_dprintf( 0, "pid: %d stack_len: %d exiting: %d",
         pid, task->proc.stack_len, new_ipc );
      adhd_task_kill( pid );
      return;
   } else {
      task->proc.ipc = new_ipc;     
   }
}

void adhd_task_kill( TASK_PID pid ) {
   if( 0 > pid || pid >= ADHD_TASKS_MAX || 0 == g_tasks[pid].proc.ipc ) {
      /* Invalid task index. */
      return;
   }

   /* Go through memory and remove any allocated blocks for this PID. */
   mfree_all( pid );

   adhd_file_close( g_tasks[pid].file_id );
   
   g_tasks[pid].proc.ipc = 0;
}

