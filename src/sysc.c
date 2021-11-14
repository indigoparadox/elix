
static SIPC_PTR vm_sysc_puts( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   SFILEPTR_T ipc_offset = 0,
      bytes_read = 0;
   unsigned char cbuf = 0;

   /* TODO: Use actual printf w/ format strings. */
   ipc_offset = vm_stack_dpop( task );
   if( 0 > ipc_offset ) { return -1; }
   if( !(task->flags & ADHD_TASK_FLAG_FOREGROUND) ) {
      return task->ipc + 1;
   }
   bytes_read = mfat_get_dir_entry_data(
      task->file_offset,
      ipc_offset,
      &cbuf, 1,
      task->disk_id, task->part_id );
   if( 0 == bytes_read ) {
      tprintf( "%s", gc_stream_error );
      return -1;
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
         tprintf( "%s", gc_stream_error );
         return -1;
      }
   }

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_mfat( TASK_PID pid, uint8_t call_id ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int32_t offset = 0;
   int16_t disk_id = 0,
      part_id = 0;
   char* filename = NULL;
   int16_t mid = 0;

   part_id = vm_stack_pop( task );
   if( 0 > part_id ) { return -1; }
   disk_id = vm_stack_pop( task );
   if( 0 > disk_id ) { return -1; }

   switch( call_id ) {
   case VM_SYSC_DROOT:
      offset = mfat_get_root_dir_offset( disk_id, part_id );
#if USE_VM_MONITOR
      printf( "droot out: %d\n", offset );
#endif /* USE_VM_MONITOR */
      break;

   case VM_SYSC_DFIRST:
      offset = vm_stack_dpop( task );
      if( 0 > offset ) { return -1; }
#if USE_VM_MONITOR
      printf( "dfirst in: %d\n", offset );
#endif /* USE_VM_MONITOR */
      offset = mfat_get_dir_entry_first_offset( offset, disk_id, part_id );
#if USE_VM_MONITOR
      printf( "dfirst out: %d\n", offset );
#endif /* USE_VM_MONITOR */
      break;

   case VM_SYSC_DNEXT:
      offset = vm_stack_dpop( task );
      if( 0 > offset ) { return -1; }
#if USE_VM_MONITOR
      printf( "dnext in: %d\n", offset );
#endif /* USE_VM_MONITOR */
      offset = mfat_get_dir_entry_next_offset( offset, disk_id, part_id );
#if USE_VM_MONITOR
      printf( "dnext out: %d\n", offset );
#endif /* USE_VM_MONITOR */
      break;

   case VM_SYSC_DNAME:
      mid = vm_stack_dpop( task );
      if( 0 > mid ) { return -1; }
      filename = mget( pid, mid, 0 );
      if( NULL == filename ) { return -1; }
      offset = vm_stack_dpop( task );
      if( 0 > offset ) { return -1; }
      mfat_get_dir_entry_name( filename, offset, disk_id, part_id );
      break;

   case VM_SYSC_LAUNCH:
      offset = vm_stack_dpop( task );
      if( 0 > offset ) { return -1; }
      adhd_task_launch( disk_id, part_id, offset );
      break;
   }

   if( 0 > vm_stack_dpush( task, offset ) ) {
      return -1;
   }

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_flagon( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t flag = 0;

   flag = vm_stack_pop( task );
   if( 0 > flag ) { return -1; }

   task->flags |= flag;

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_flagoff( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t flag = 0;

   flag = vm_stack_pop( task );
   if( 0 > flag ) { return -1; }

   task->flags &= ~flag;

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_putc( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int16_t c = 0;

   c = vm_stack_pop( task );
   if( 0 > c ) { return -1; }
   if( task->flags & ADHD_TASK_FLAG_FOREGROUND ) {
      tputc( c );
   }

   return task->ipc + 1;
}

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

