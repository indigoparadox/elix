
#include "uprintf.h"
#include "adhd.h"
#define SYSC_C
#include "sysc.h"
#include "mem.h"
#include "console.h"

VM_SIPC sysc_NOOP( TASK_PID pid, uint8_t flags ) {
}

VM_SIPC sysc_PUTS( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   SFILEPTR_T ipc_offset = 0,
      bytes_read = 0;
   unsigned char cbuf = 0;

   /* TODO: Use actual printf w/ format strings. */
   ipc_offset = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == ipc_offset ) { return VM_ERROR_STACK; }
   if( !(task->flags & ADHD_TASK_FLAG_FOREGROUND) ) {
      return task->proc.ipc + 4;
   }
#ifdef USE_DISK
   bytes_read = mfat_get_dir_entry_data(
      g_files[task->file_id].offset,
      ipc_offset,
      &cbuf, 1,
      g_files[task->file_id].disk_id, g_files[task->file_id].part_id );
   if( 0 == bytes_read ) {
      return SYSC_ERROR_DISK;
   }
   while( 0 < cbuf ) {
      tputc( cbuf );
      ipc_offset += 1;
      bytes_read = mfat_get_dir_entry_data(
         g_files[task->file_id].offset,
         ipc_offset,
         &cbuf, 1,
         g_files[task->file_id].disk_id, g_files[task->file_id].part_id );
      if( 0 == bytes_read ) {
         return SYSC_ERROR_DISK;
      }
   }
#else
   /* TODO: VM without disk strings? */
#endif

   return task->proc.ipc + 4;
}

#ifdef USE_DISK

VM_SIPC sysc_DROOT( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t disk_id = 0,
      part_id = 0;
   uint32_t offset = 0;
   int8_t file_id = 0;

   part_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == part_id ) { return VM_ERROR_STACK; }
   disk_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == disk_id ) { return VM_ERROR_STACK; }

   offset = mfat_get_root_dir_offset( disk_id, part_id );
   assert( 0 <= offset );
   file_id = adhd_file_open( disk_id, part_id, offset, ADHD_FILE_FLAG_DIR );
   assert( 0 <= file_id );

   elix_dprintf( 1, "root directory %d opened offset: %d\n", file_id, offset );

   if( VM_ERROR_STACK == vm_op_SPUSH( &(task->proc), flags, file_id ) ) {
      return VM_ERROR_STACK;
   }

   return task->proc.ipc + 4;
}

VM_SIPC sysc_DFIRST( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t disk_id = 0,
      part_id = 0,
      file_id = 0;
   uint32_t dir_offset = 0,
      entry_offset = 0;

   file_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == file_id ) { return VM_ERROR_STACK; }

   /* Get the offset of the first entry to iterate from. */
   dir_offset = g_files[file_id].offset;
   assert( 0 <= dir_offset );
   disk_id = g_files[file_id].disk_id;
   part_id = g_files[file_id].part_id;
   adhd_file_close( file_id );

   entry_offset = mfat_get_dir_entry_first_offset(
      dir_offset, disk_id, part_id );
   assert( 0 <= entry_offset );
   file_id = adhd_file_open( disk_id, part_id, entry_offset, 0 );
   assert( 0 <= file_id );

   elix_dprintf(
      1, "first entry %d opened offset: %d\n", file_id, entry_offset );

   if( VM_ERROR_STACK == vm_op_SPUSH( &(task->proc), flags, file_id ) ) {
      return VM_ERROR_STACK;
   }

   return task->proc.ipc + 4;
}

VM_SIPC sysc_DNEXT( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t disk_id = 0,
      part_id = 0,
      file_id = 0;
   uint32_t offset = 0;

   file_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == offset ) { return VM_ERROR_STACK; }

   /* Get the offset of the last entry to iterate from. */
   offset = g_files[file_id].offset;
   assert( 0 <= offset );
   disk_id = g_files[file_id].disk_id;
   part_id = g_files[file_id].part_id;
   adhd_file_close( file_id );

   offset = mfat_get_dir_entry_next_offset( offset, disk_id, part_id );
   assert( 0 <= offset );
   file_id = adhd_file_open( disk_id, part_id, offset, 0 );
   assert( 0 <= file_id );

   elix_dprintf( 1, "next entry %d opened offset: %d\n", file_id, offset );

   if( VM_ERROR_STACK == vm_op_SPUSH( &(task->proc), flags, file_id ) ) {
      return VM_ERROR_STACK;
   }

   return task->proc.ipc + 4;
}

VM_SIPC sysc_DNAME( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   char* filename = NULL;
   int16_t disk_id = 0,
      part_id = 0,
      mid = 0,
      file_id = 0;
   uint32_t offset = 0;

   mid = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == mid ) { return VM_ERROR_STACK; }
   file_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == offset ) { return VM_ERROR_STACK; }

   filename = mget( pid, mid, 0 );
   if( NULL == filename ) { return SYSC_ERROR_MEM; }

   offset = g_files[file_id].offset;
   assert( 0 <= offset );
   disk_id = g_files[file_id].disk_id;
   part_id = g_files[file_id].part_id;
   mfat_get_dir_entry_name( filename, offset, disk_id, part_id );
   /* adhd_file_close( file_id ); */

   return task->proc.ipc + 4;
}

VM_SIPC sysc_DENTRY( TASK_PID pid, uint8_t flags ) {
   /* TODO */
   return SYSC_ERROR_UNIMPLEMENTED;
}

VM_SIPC sysc_LAUNCH( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t file_id = 0,
      launch_pid = 0;

   file_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == file_id ) { return VM_ERROR_STACK; }

   launch_pid = adhd_task_launch( file_id );

   if( VM_ERROR_STACK == vm_op_SPUSH( &(task->proc), flags, launch_pid ) ) {
      return VM_ERROR_STACK;
   }

   return task->proc.ipc + 4;
}

#else

VM_SIPC sysc_DROOT( TASK_PID pid, uint8_t flags ) {
}

VM_SIPC sysc_DFIRST( TASK_PID pid, uint8_t flags ) {
}

VM_SIPC sysc_DNEXT( TASK_PID pid, uint8_t flags ) {
}

VM_SIPC sysc_DNAME( TASK_PID pid, uint8_t flags ) {
}

VM_SIPC sysc_DENTRY( TASK_PID pid, uint8_t flags ) {
}

VM_SIPC sysc_LAUNCH( TASK_PID pid, uint8_t flags ) {
}

#endif /* USE_DISK */

VM_SIPC sysc_FLAGON( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t flag = 0;

   flag = vm_op_POP( &(task->proc), flags, 0 );
   if( 0 > flag ) { return -1; }

   task->flags |= flag;

   return task->proc.ipc + 4;
}

VM_SIPC sysc_FLAGOFF( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t flag = 0;

   flag = vm_op_POP( &(task->proc), flags, 0 );
   if( 0 > flag ) { return -1; }

   task->flags &= ~flag;

   return task->proc.ipc + 4;
}

VM_SIPC sysc_PUTC( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t c = 0;

   c = vm_op_POP( &(task->proc), flags, 0 );
   if( 0 > c ) { return -1; }
   if( task->flags & ADHD_TASK_FLAG_FOREGROUND ) {
      tputc( c );
   }

   return task->proc.ipc + 4;
}

VM_SIPC sysc_EXIT( TASK_PID pid, uint8_t flags ) {
   return SYSC_ERROR_EXIT;
}

VM_SIPC sysc_GETC( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   unsigned char cbuf = 0;

   if( task->flags & ADHD_TASK_FLAG_FOREGROUND ) {
      cbuf = tgetc();
      if( VM_ERROR_STACK == vm_op_SPUSH( &(task->proc), flags, cbuf ) ) {
         return VM_ERROR_STACK;
      }
   } else {
      if( VM_ERROR_STACK == vm_op_SPUSH( &(task->proc), flags, 0 ) ) {
         return VM_ERROR_STACK;
      }
   }

   return task->proc.ipc + 4;
}

VM_SIPC sysc_MPUTS( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t mid = 0;
   char* str_ptr = NULL;

   mid = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == mid ) { return VM_ERROR_STACK; }

   if( task->flags & ADHD_TASK_FLAG_FOREGROUND ) {
      str_ptr = mget( pid, mid, 0 );
      while( '\0' != *str_ptr ) {
         tputc( *str_ptr );
         str_ptr++;
      }
   }

   return task->proc.ipc + 4;
}

VM_SIPC sysc_CMP( TASK_PID pid, uint8_t flags ) {
   /* TODO */
   return SYSC_ERROR_UNIMPLEMENTED;
}

VM_SIPC sysc_ICMP( TASK_PID pid, uint8_t flags ) {
   /* TODO */
   return SYSC_ERROR_UNIMPLEMENTED;
}

#if 0

static SIPC_PTR vm_instr_sysc( TASK_PID pid, uint8_t call_id ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   unsigned char cbuf = 0;
   char* str_ptr = NULL;
   int16_t mid = 0;

   assert( 0 <= pid );
   assert( 0 <= task->ipc );

   switch( call_id ) {
   case VM_SYSC_EXIT:
      return -1;

   case VM_SYSC_PUTC:
      return vm_sysc_putc( pid );

   case VM_SYSC_GETC:
      break;

   case VM_SYSC_MPUTS:

   case VM_SYSC_PUTS:
      return vm_sysc_puts( pid );

   case VM_SYSC_FLAGON:
      return vm_sysc_flagon( pid );

   case VM_SYSC_FLAGOFF:
      return vm_sysc_flagoff( pid );

   case VM_SYSC_LAUNCH:
      return vm_sysc_mfat( pid, call_id );

   case VM_SYSC_DROOT:
      return vm_sysc_mfat( pid, call_id );
      
   case VM_SYSC_DFIRST:
      return vm_sysc_mfat( pid, call_id );
      
   case VM_SYSC_DNEXT:
      return vm_sysc_mfat( pid, call_id );
      
   case VM_SYSC_DNAME:
      return vm_sysc_mfat( pid, call_id );

   }

   return task->ipc + 4;
}

#endif

VM_SIPC sysc_MALLOC( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t mid = 0,
      sz = 0;
   uint8_t* addr_tmp = NULL;

   mid = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == mid ) { return VM_ERROR_STACK; }
   sz = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == sz ) { return VM_ERROR_STACK; }

   addr_tmp = mget( pid, mid, sz );

   /* Not NULL or offset of data from NULL.*/
   if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
      return SYSC_ERROR_MEM;
   }

   return task->proc.ipc + 4;
}

VM_SIPC sysc_MPOP( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t mid = 0,
      buf = 0,
      offset = 0;
   int16_t* addr_tmp = NULL;

   mid = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == mid ) { return VM_ERROR_STACK; }
   offset = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == mid ) { return VM_ERROR_STACK; }
   buf = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == mid ) { return VM_ERROR_STACK; }

   addr_tmp = mget( pid, mid, 0 );

   /* Not NULL or offset of data from NULL.*/
   if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
      return SYSC_ERROR_MEM;
   }

   *(addr_tmp + offset) = buf;

   return task->proc.ipc + 4;
}

VM_SIPC sysc_MPUSH( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t mid = 0,
      buf = 0,
      offset = 0;
   int16_t* addr_tmp = NULL;

   mid = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == mid ) { return VM_ERROR_STACK; }
   offset = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == mid ) { return VM_ERROR_STACK; }

   addr_tmp = mget( pid, mid, 0 );
   /* Not NULL or offset of data from NULL.*/
   if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
      return SYSC_ERROR_MEM;
   }

   if( VM_ERROR_STACK == vm_op_SPUSH(
      &(task->proc), flags, *(addr_tmp + offset) )
   ) {
      return VM_ERROR_STACK;
   }

   return task->proc.ipc + 4;
}

VM_SIPC sysc_MFREE( TASK_PID pid, uint8_t flags ) {
   struct ADHD_TASK* task = &(g_tasks[pid]);
   int16_t mid = 0,
      buf = 0,
      offset = 0;
   int16_t* addr_tmp = NULL;

   mid = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == mid ) { return VM_ERROR_STACK; }
 
   elix_dprintf( 1, "freeing mid: %d\n", mid );

   mfree( pid, mid );

   return task->proc.ipc + 4;
}

