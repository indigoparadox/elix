
#include "adhd.h"
#define SYSC_C
#include "sysc.h"

VM_SIPC sysc_NOOP( TASK_PID pid, uint8_t flags ) {
}

VM_SIPC sysc_PUTS( TASK_PID pid, uint8_t flags ) {
   struct adhd_task* task = &(g_tasks[pid]);
   SFILEPTR_T ipc_offset = 0,
      bytes_read = 0;
   unsigned char cbuf = 0;

   /* TODO: Use actual printf w/ format strings. */
   ipc_offset = vm_op_POP( &(task->proc), flags, 0 );
   if( 0 > ipc_offset ) { return -1; }
   if( !(task->flags & ADHD_TASK_FLAG_FOREGROUND) ) {
      return task->proc.ipc + 1;
   }
   bytes_read = mfat_get_dir_entry_data(
      task->file_offset,
      ipc_offset,
      &cbuf, 1,
      task->disk_id, task->part_id );
   if( 0 == bytes_read ) {
      return SYSC_ERROR_DISK;
   }
   while( 0 < cbuf ) {
      tputc( cbuf );
      ipc_offset += 1;
      bytes_read = mfat_get_dir_entry_data(
         task->file_offset,
         ipc_offset,
         &cbuf, 1,
         task->disk_id, task->part_id );
      if( 0 == bytes_read ) {
         return SYSC_ERROR_DISK;
      }
   }

   return task->proc.ipc + 1;
}

VM_SIPC sysc_DROOT( TASK_PID pid, uint8_t flags ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t disk_id = 0,
      part_id = 0,
      offset = 0;

   part_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == part_id ) { return VM_ERROR_STACK; }
   disk_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == disk_id ) { return VM_ERROR_STACK; }

   offset = mfat_get_root_dir_offset( disk_id, part_id );

   vm_dprintf( 0, "droot out: %d", offset );

   if( VM_ERROR_STACK == vm_op_PUSH( &(task->proc), flags, offset ) ) {
      return VM_ERROR_STACK;
   }

   return task->proc.ipc + 1;
}

VM_SIPC sysc_DFIRST( TASK_PID pid, uint8_t flags ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t disk_id = 0,
      part_id = 0,
      offset = 0;

   part_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == part_id ) { return VM_ERROR_STACK; }
   disk_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == disk_id ) { return VM_ERROR_STACK; }
   offset = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == offset ) { return VM_ERROR_STACK; }

   vm_dprintf( 0, "dfirst in: %d", offset );
   offset = mfat_get_dir_entry_first_offset( offset, disk_id, part_id );
   vm_dprintf( 0, "dfirst out: %d", offset );

   if( VM_ERROR_STACK == vm_op_PUSH( &(task->proc), flags, offset ) ) {
      return VM_ERROR_STACK;
   }

   return task->proc.ipc + 1;
}

VM_SIPC sysc_DNEXT( TASK_PID pid, uint8_t flags ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t disk_id = 0,
      part_id = 0,
      offset = 0;

   part_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == part_id ) { return VM_ERROR_STACK; }
   disk_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == disk_id ) { return VM_ERROR_STACK; }
   offset = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == offset ) { return VM_ERROR_STACK; }

   vm_dprintf( 0, "dnext in: %d", offset );
   offset = mfat_get_dir_entry_next_offset( offset, disk_id, part_id );
   vm_dprintf( 0, "dnext out: %d", offset );

   if( VM_ERROR_STACK == vm_op_PUSH( &(task->proc), flags, offset ) ) {
      return VM_ERROR_STACK;
   }

   return task->proc.ipc + 1;
}

VM_SIPC sysc_DNAME( TASK_PID pid, uint8_t flags ) {
   struct adhd_task* task = &(g_tasks[pid]);
   char* filename = NULL;
   int16_t disk_id = 0,
      part_id = 0,
      offset = 0,
      mid = 0;

   part_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == part_id ) { return VM_ERROR_STACK; }
   disk_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == disk_id ) { return VM_ERROR_STACK; }
   offset = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == offset ) { return VM_ERROR_STACK; }

   mid = vm_stack_dpop( task );
   if( VM_ERROR_STACK == mid ) { return VM_ERROR_STACK; }
   filename = mget( pid, mid, 0 );
   if( NULL == filename ) { return SYSC_ERROR_MEM; }
   offset = vm_stack_dpop( task );
   if( VM_ERROR_STACK == offset ) { return VM_ERROR_STACK; }

   mfat_get_dir_entry_name( filename, offset, disk_id, part_id );

   return task->proc.ipc + 1;
}

VM_SIPC sysc_LAUNCH( TASK_PID pid, uint8_t flags ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t disk_id = 0,
      part_id = 0,
      offset = 0;

   part_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == part_id ) { return VM_ERROR_STACK; }
   disk_id = vm_op_POP( &(task->proc), flags, 0 );
   if( VM_ERROR_STACK == disk_id ) { return VM_ERROR_STACK; }
   offset = vm_stack_dpop( task );
   if( VM_ERROR_STACK == offset ) { return VM_ERROR_STACK; }

   adhd_task_launch( disk_id, part_id, offset );

   if( VM_ERROR_STACK == vm_op_PUSH( &(task->proc), flags, offset ) ) {
      return VM_ERROR_STACK;
   }

   return task->proc.ipc + 1;
}

VM_SIPC sysc_FLAGON( TASK_PID pid, uint8_t flags ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t flag = 0;

   flag = vm_op_POP( &(task->proc), flags, 0 );
   if( 0 > flag ) { return -1; }

   task->flags |= flag;

   return task->proc.ipc + 1;
}

VM_SIPC sysc_FLAGOFF( TASK_PID pid, uint8_t flags ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t flag = 0;

   flag = vm_op_POP( &(task->proc), flags, 0 );
   if( 0 > flag ) { return -1; }

   task->flags &= ~flag;

   return task->proc.ipc + 1;
}

VM_SIPC sysc_PUTC( TASK_PID pid, uint8_t flags ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t c = 0;

   c = vm_op_POP( &(task->proc), flags, 0 );
   if( 0 > c ) { return -1; }
   if( task->flags & ADHD_TASK_FLAG_FOREGROUND ) {
      tputc( c );
   }

   return task->proc.ipc + 1;
}

#if 0

static SIPC_PTR vm_instr_sysc( TASK_PID pid, uint8_t call_id ) {
   struct adhd_task* task = &(g_tasks[pid]);
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
      if( task->flags & ADHD_TASK_FLAG_FOREGROUND ) {
         cbuf = tgetc();
         if( 0 > vm_stack_push( task, cbuf ) ) {
            return -1;
         }
      } else {
         if( 0 > vm_stack_push( task, 0 ) ) {
            return -1;
         }
      }
      break;

   case VM_SYSC_MPUTS:
      mid = vm_stack_dpop( task );
      if( 0 > mid ) { return -1; }
      if( task->flags & ADHD_TASK_FLAG_FOREGROUND ) {
         str_ptr = mget( pid, mid, 0 );
         while( '\0' != *str_ptr ) {
            tputc( *str_ptr );
            str_ptr++;
         }
      }
      break;

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

   return task->ipc + 1;
}

#endif

#if 0

static ssize_t vm_instr_mem( TASK_PID pid, uint8_t instr, MEMLEN_T mid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t* addr_tmp = NULL;
   SMEMLEN_T sz_or_offset = 0;
   int32_t buf = 0;

   switch( instr ) {
   case VM_INSTR_MALLOC:
      sz_or_offset = vm_stack_dpop( task );
      if( 0 > sz_or_offset ) { return -1; }
      addr_tmp = mget( pid, mid, sz_or_offset );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tprintf( "%s", gc_mem_error );
         return -1;
      }
      break;

   case VM_INSTR_MPOP:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tprintf( "%s", gc_mem_error );
         return -1;
      }
      buf = vm_stack_pop( task );
      if( 0 > buf ) { return -1; }
      *addr_tmp = buf;
      break;

   case VM_INSTR_MPOPD:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      /* TODO: Verify memory sz is >=2 */
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tprintf( "%s", gc_mem_error );
         return -1;
      }
      buf = vm_stack_dpop( task );
      if( 0 > buf ) { return -1; }
      *((uint16_t*)addr_tmp) = buf;
#if USE_VM_MONITOR
      printf( "popped to memory: %d\n", *((uint16_t*)addr_tmp) );
#endif /* USE_VM_MONITOR */
      break;

   case VM_INSTR_MPOPO:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tprintf( "%s", gc_mem_error );
         return -1;
      }
      sz_or_offset = vm_stack_dpop( task );
      if( 0 > sz_or_offset ) { return -1; }
      buf = vm_stack_pop( task );
      if( 0 > buf ) { return -1; }
      *(addr_tmp + sz_or_offset) = buf;
      break;

   case VM_INSTR_MPUSHC:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tprintf( "%s", gc_mem_error );
         return -1;
      }
      if( 0 > vm_stack_push( task, *addr_tmp ) ) {
         return -1;
      }
      break;

   case VM_INSTR_MPUSHCD:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      /* TODO: Verify memory sz is >=2 */
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tprintf( "%s", gc_mem_error );
         return -1;
      }
      if( 0 > vm_stack_dpush( task, *((uint16_t*)addr_tmp) ) ) {
         return -1;
      }
      break;

   case VM_INSTR_MPUSHCO:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tprintf( "%s", gc_mem_error );
         return -1;
      }
      sz_or_offset = vm_stack_dpop( task );
      if( 0 > sz_or_offset ) { return -1; }
      if( 0 > vm_stack_push( task, *(addr_tmp + sz_or_offset) ) ) {
         return -1;
      }
#if USE_VM_MONITOR
      printf( "pushed: %d (offset %d)\n",
         *(addr_tmp + sz_or_offset), sz_or_offset );
#endif /* USE_VM_MONITOR */
      break;

   case VM_INSTR_MFREE:
      mfree( pid, mid );
      break;
   }

   return task->ipc + 1;
}
#endif
