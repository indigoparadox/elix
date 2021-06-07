
#include "code16.h"

#include "vm.h"
#include "console.h"
#include "adhd.h"

//#define USE_VM_MONITOR 1

#if USE_VM_MONITOR
#include "vm_debug.h"
#endif /* USE_VM_MONITOR */

static const char gc_stack_error[] = "aborting; stack error\n";
static const char gc_mem_error[] = "aborting; mem error\n";
static const char gc_stream_error[] = "aborting; stream error\n";

static uint8_t vm_stack_pop( struct adhd_task* task, uint8_t* uf ) {
   if( 0 >= task->stack_len ) {
      tputs( gc_stack_error );
      *uf = 1;
      return 0;
   }
   task->stack_len--;
   return task->stack[task->stack_len];
}

static uint16_t vm_stack_dpop( struct adhd_task* task, uint8_t* uf ) {
   uint16_t dout = 0;
   if( 1 >= task->stack_len ) {
      tputs( gc_stack_error );
      *uf = 1;
      return 0;
   }
   task->stack_len--;
   dout |= (task->stack[task->stack_len]) & 0x00ff;
   task->stack_len--;
   dout |= (task->stack[task->stack_len] << 8) & 0xff00;
   return dout;
}

static void vm_stack_push( struct adhd_task* task, uint8_t data, uint8_t *of ) {
   if( ADHD_STACK_MAX <= task->stack_len + 1 ) {
      tputs( gc_stack_error );
      *of = 1;
      return;
   }
   task->stack[task->stack_len] = data;
   task->stack_len++;
}

static void vm_stack_dpush( struct adhd_task* task, uint16_t data, uint8_t* of ) {
   if( ADHD_STACK_MAX <= task->stack_len + 2 ) {
      tputs( gc_stack_error );
      *of = 1;
      return;
   }
   task->stack[task->stack_len] = (uint8_t)((data >> 8) & 0x00ff);
   task->stack_len++;
   task->stack[task->stack_len] = (uint8_t)(data & 0x00ff);
   task->stack_len++;
}

union vm_data_type {
   uint8_t byte;
   IPC_PTR ipc;
};

static SIPC_PTR vm_sysc_puts( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   FILEPTR_T ipc_offset = 0,
      bytes_read = 0;
   unsigned char cbuf = 0;
   uint8_t stack_error = 0;

   /* TODO: Use actual printf w/ format strings. */
   ipc_offset = vm_stack_dpop( task, &stack_error );
   if( stack_error ) { return -1; }
   if( !(task->flags & ADHD_TASK_FLAG_FOREGROUND) ) {
      return task->ipc + 1;
   }
   bytes_read = mfat_get_dir_entry_data(
      task->file_offset,
      ipc_offset,
      &cbuf, 1,
      task->disk_id, task->part_id );
   if( 0 == bytes_read ) {
      tputs( gc_stream_error );
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
         tputs( gc_stream_error );
         return -1;
      }
   }

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_droot( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint16_t offset = 0;
   uint8_t disk_id = 0,
      part_id = 0;
   uint8_t stack_error = 0;

   disk_id = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }
   part_id = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }

   offset = mfat_get_root_dir_offset( disk_id, part_id );

   vm_stack_dpush( task, offset, &stack_error );
   if( stack_error ) {
      return -1;
   }

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_dfirst( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint16_t offset = 0;
   uint8_t disk_id = 0,
      part_id = 0;
   uint8_t stack_error = 0;

   part_id = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }
   disk_id = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }
   offset = vm_stack_dpop( task, &stack_error );
   if( stack_error ) { return -1; }

   offset = mfat_get_dir_entry_first_offset( offset, disk_id, part_id );

   vm_stack_dpush( task, offset, &stack_error );
   if( stack_error ) { return -1; }

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_dnext( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint16_t offset = 0;
   uint8_t disk_id = 0,
      part_id = 0;
   uint8_t stack_error = 0;

   part_id = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }
   disk_id = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }
   offset = vm_stack_dpop( task, &stack_error );
   if( stack_error ) { return -1; }

   offset = mfat_get_dir_entry_next_offset( offset, disk_id, part_id );

   vm_stack_dpush( task, offset, &stack_error );
   if( stack_error ) { return -1; }

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_dname( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint16_t offset = 0;
   uint8_t disk_id = 0,
      part_id = 0;
   char* filename = NULL;
   uint8_t stack_error = 0;
   uint8_t mid = 0;

   mid = vm_stack_dpop( task, &stack_error );
   if( stack_error ) { return -1; }
   filename = mget( pid, mid, 0 );
   offset = vm_stack_dpop( task, &stack_error );
   if( stack_error ) { return -1; }
   part_id = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }
   disk_id = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }

   mfat_get_dir_entry_name( filename, offset, disk_id, part_id );

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_icmp( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   char* cmp1 = NULL,
      * cmp2 = NULL;
   STRLEN_T res = 0;
   uint8_t len = 0;
   char sep = ' ';
   uint8_t stack_error = 0;
   uint8_t mid = 0;

   sep = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }
   len = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }

   mid = vm_stack_dpop( task, &stack_error );
   if( stack_error ) { return -1; }
   cmp1 = mget( pid, mid, 0 );

   mid = vm_stack_dpop( task, &stack_error );
   if( stack_error ) { return -1; }
   cmp2 = mget( pid, mid, 0 );

   res = alpha_cmp_cc( cmp1, len, cmp2, len, sep, false, false );

   vm_stack_push( task, (uint8_t)res, &stack_error );
   if( stack_error ) { return -1; }

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_launch( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t disk_id = 0,
      part_id = 0;
   FILEPTR_T offset = 0;
   uint8_t stack_error = 0;

   offset = vm_stack_dpop( task, &stack_error );
   if( stack_error ) { return -1; }
   part_id = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }
   disk_id = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }

   adhd_task_launch( disk_id, part_id, offset );

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_flagon( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t flag = 0;
   uint8_t stack_error = 0;

   flag = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }

   task->flags |= flag;

   return task->ipc + 1;
}

static SIPC_PTR vm_sysc_flagoff( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t flag = 0;
   uint8_t stack_error = 0;

   flag = vm_stack_pop( task, &stack_error );
   if( stack_error ) { return -1; }

   task->flags &= ~flag;

   return task->ipc + 1;
}

static SIPC_PTR vm_instr_sysc( TASK_PID pid, uint8_t call_id ) {
   struct adhd_task* task = &(g_tasks[pid]);
   union vm_data_type data;
   unsigned char cbuf = 0;
   char* str_ptr = NULL;
   uint8_t stack_error = 0;

   assert( 0 <= pid );
   assert( 0 <= task->ipc );

   switch( call_id ) {
   case VM_SYSC_EXIT:
      return -1;

   case VM_SYSC_PUTC:
      data.byte = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      if( task->flags & ADHD_TASK_FLAG_FOREGROUND ) {
         tputc( data.byte );
      }
      break;

   case VM_SYSC_GETC:
      if( task->flags & ADHD_TASK_FLAG_FOREGROUND ) {
         cbuf = tgetc();
         vm_stack_push( task, cbuf, &stack_error );
         if( stack_error ) { return -1; }
      } else {
         vm_stack_push( task, 0, &stack_error );
         if( stack_error ) { return -1; }
      }
      break;

   case VM_SYSC_MPUTS:
      data.ipc = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      if( task->flags & ADHD_TASK_FLAG_FOREGROUND ) {
         str_ptr = mget( pid, data.ipc, 0 );
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
      return vm_sysc_launch( pid );

   case VM_SYSC_DROOT:
      return vm_sysc_droot( pid );
      
   case VM_SYSC_DFIRST:
      return vm_sysc_dfirst( pid );
      
   case VM_SYSC_DNEXT:
      return vm_sysc_dnext( pid );
      
   case VM_SYSC_DNAME:
      return vm_sysc_dname( pid );

   case VM_SYSC_ICMP:
      return vm_sysc_icmp( pid );
   }

   return task->ipc + 1;
}

static SIPC_PTR vm_instr_branch( TASK_PID pid, uint8_t instr, IPC_PTR addr ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t comp1 = 0,
      comp2 = 0;
   uint16_t dcomp1 = 0,
      dcomp2 = 0;
   IPC_PTR retval = task->ipc + 1;
   uint8_t stack_error = 0;
   
   switch( instr ) {
   case VM_INSTR_JUMP:
      return addr;

#if 0
   case VM_INSTR_JSNZ:
      comp1 = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      if( 0 != comp1 ) {
         retval = addr;
      }
      vm_stack_push( task, comp1, &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_JSZ:
      comp1 = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      if( 0 == comp1 ) {
         retval = addr;
      }
      vm_stack_push( task, comp1, &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_JSZD:
      dcomp1 = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      if( 0 == dcomp1 ) {
         retval = addr;
      }
      vm_stack_dpush( task, dcomp1, &stack_error );
      if( stack_error ) { return -1; }
      break;
#endif

   case VM_INSTR_JSEQ:
      comp2 = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      comp1 = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      if( comp1 == comp2 ) {
         retval = addr;
      }
      vm_stack_push( task, comp1, &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_JSNED:
      dcomp2 = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      dcomp1 = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      if( dcomp1 != dcomp2 ) {
         retval = addr;
      }
      vm_stack_push( task, dcomp1, &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_JSNE:
      comp2 = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      comp1 = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      if( comp1 != comp2 ) {
         retval = addr;
      }
      vm_stack_push( task, comp1, &stack_error );
      if( stack_error ) { return -1; }
      break;

#if 0
   case VM_INSTR_JSGE:
      comp2 = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      comp1 = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      if( comp1 >= comp2 ) {
         retval = addr;
      }
      vm_stack_push( task, comp1, &stack_error );
      if( stack_error ) { return -1; }
      break;
#endif

   case VM_INSTR_JSGED:
      dcomp2 = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      dcomp1 = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      if( dcomp1 >= dcomp2 ) {
         retval = addr;
      }
      vm_stack_dpush( task, dcomp1, &stack_error );
      if( stack_error ) { return -1; }
      break;
   }

   return retval;
}

static ssize_t vm_instr_mem( TASK_PID pid, uint8_t instr, MEMLEN_T mid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t* addr_tmp = NULL;
   MEMLEN_T offset = 0;
   MEMLEN_T sz = 0;
   uint8_t stack_error = 0;

   switch( instr ) {
   case VM_INSTR_MALLOC:
      sz = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      addr_tmp = mget( pid, mid, sz );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tputs( gc_mem_error );
         return -1;
      }
      break;

   case VM_INSTR_MPOP:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tputs( gc_mem_error );
         return -1;
      }
      *addr_tmp = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_MPOPC:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tputs( gc_mem_error );
         return -1;
      }
      *addr_tmp = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      vm_stack_push( task, *addr_tmp, &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_MPOPD:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      /* TODO: Verify memory sz is >=2 */
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tputs( gc_mem_error );
         return -1;
      }
      *((uint16_t*)addr_tmp) = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_MPOPCD:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      /* TODO: Verify memory sz is >=2 */
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tputs( gc_mem_error );
         return -1;
      }
      *((uint16_t*)addr_tmp) = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      vm_stack_dpush( task, *((uint16_t*)addr_tmp), &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_MPOPCO:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tputs( gc_mem_error );
         return -1;
      }
      offset = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      *(addr_tmp + offset) = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      vm_stack_push( task, *(addr_tmp + offset), &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_MPOPO:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tputs( gc_mem_error );
         return -1;
      }
      offset = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      *(addr_tmp + offset) = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_MPUSHC:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tputs( gc_mem_error );
         return -1;
      }
      vm_stack_push( task, *addr_tmp, &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_MPUSHCD:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      /* TODO: Verify memory sz is >=2 */
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tputs( gc_mem_error );
         return -1;
      }
      vm_stack_dpush( task, *((uint16_t*)addr_tmp), &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_MPUSHCO:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      if( NULL == addr_tmp || (void*)0x4 == addr_tmp ) {
         tputs( gc_mem_error );
         return -1;
      }
      offset = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      vm_stack_push( task, *(addr_tmp + offset), &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_MFREE:
      mfree( pid, mid );
      break;
   }

   return task->ipc + 1;
}

static SIPC_PTR vm_instr_stack( TASK_PID pid, uint8_t instr ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint16_t data1 = 0,
      data2 = 0;
   uint8_t stack_error = 0;
   SIPC_PTR retval = task->ipc + 1;

   switch( instr ) {
   case VM_INSTR_SJUMP:
      retval = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_SPOP:
      vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      break;

#if 0
   case VM_INSTR_SDPOP:
      vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      break;

   case VM_INSTR_SADD:
      data1 = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      data2 = vm_stack_pop( task, &stack_error );
      if( stack_error ) { return -1; }
      vm_stack_push( task, data1 + data2, &stack_error );
      if( stack_error ) { return -1; }
      break;
#endif

   case VM_INSTR_SADDD:
      data1 = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      data2 = vm_stack_dpop( task, &stack_error );
      if( stack_error ) { return -1; }
      vm_stack_dpush( task, data1 + data2, &stack_error );
      if( stack_error ) { return -1; }
      break;
   }

   return retval;
}

SIPC_PTR vm_instr_execute( TASK_PID pid, uint16_t instr_full ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t instr = instr_full >> 8;
   uint8_t data = instr_full & 0xff;
   uint16_t ddata = 0;
   uint8_t stack_error = 0;

   assert( 0 <= pid );
   assert( 0 <= task->ipc );

#if USE_VM_MONITOR
   if( task->prev_instr ) {
      printf( "pid: %d, ipc: %d, double data: %d (0x%x), stack: %d\n",
         pid, task->ipc, instr_full, instr_full, task->stack_len );
   } else {
      int j = 0, k = 0;
      while(
        -1  != vm_instr_debug[j].val &&
        ((task->prev_instr && task->prev_instr != vm_instr_debug[j].val) ||
        instr != vm_instr_debug[j].val)
      ) {
         j++;
      }
      if( VM_INSTR_SYSC == instr ) {
         while(
         -1  != vm_sysc_debug[k].val && data != vm_sysc_debug[k].val
         ) {
            k++;
         }
         printf(
            "pid: %d, ipc: %d, instr: %s (%d) (0x%x), sysc: %s (%d) (0x%x), stack: %d\n",
            pid,
            task->ipc,
            vm_instr_debug[j].name,
            instr,
            instr,
            vm_sysc_debug[k].name,
            data,
            data,
            task->stack_len );
      } else {
         printf(
            "pid: %d, ipc: %d, instr: %s (%d) (0x%x), data: %d (0x%x), stack: %d\n",
            pid,
            task->ipc,
            vm_instr_debug[j].name,
            instr,
            instr,
            data,
            data,
            task->stack_len );
      }
   }

   assert( task->prev_instr || 0 != instr );
#endif /* USE_VM_MONITOR */

   /* Process instructions with double parameters if this is 2nd cycle. */
   if( VM_INSTR_PUSHD == task->prev_instr ) {
      task->prev_instr = 0;
      /* instr_full is double data, here. */
      vm_stack_dpush( task, instr_full, &stack_error );
      if( stack_error ) { return -1; }
      return task->ipc + 1; /* Done processing. */
   
   } else if(
      VM_INSTR_MMIN <= task->prev_instr && VM_INSTR_MMAX >= task->prev_instr
   ) {
      ddata = task->prev_instr;
      task->prev_instr = 0;
      /* instr_full is double data, here. */
      return vm_instr_mem( pid, ddata, instr_full );

   } else if(
      VM_INSTR_JMIN <= task->prev_instr && VM_INSTR_JMAX >= task->prev_instr
   ) {
      ddata = task->prev_instr;
      task->prev_instr = 0;

      /* instr_full is double data, here. */
      return vm_instr_branch( pid, ddata, instr_full );
   }

   /* Process normal instructions and 1st cycles. */
   if( VM_INSTR_PUSH == instr ) {
      vm_stack_push( task, data, &stack_error );
      if( stack_error ) { return -1; }

   } else if( VM_INSTR_PUSHD == instr ) {
      /* Push will happen on next execute with full number. */
      task->prev_instr = VM_INSTR_PUSHD;

   } else if( VM_INSTR_SJUMP == instr ) {
      return vm_instr_stack( pid, instr );

   } else if( VM_INSTR_SPOP == instr ) {
      return vm_instr_stack( pid, instr );

   //} else if( VM_INSTR_SDPOP == instr ) {
   //   return vm_instr_stack( pid, instr );

   //} else if( VM_INSTR_SADD == instr ) {
   //   return vm_instr_stack( pid, instr );

   } else if( VM_INSTR_SADDD == instr ) {
      return vm_instr_stack( pid, instr );

   } else if( VM_INSTR_SYSC == instr ) {
      return vm_instr_sysc( pid, data );
   
   } else if(
      VM_INSTR_JMIN <= instr && VM_INSTR_JMAX >= instr
   ) {
      task->prev_instr = instr;
   
   } else if(
      VM_INSTR_MMIN <= instr && VM_INSTR_MMAX >= instr
   ) {
      task->prev_instr = instr;
   }

   return task->ipc + 1;
}

