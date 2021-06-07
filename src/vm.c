
#include "code16.h"

#include "vm.h"
#include "console.h"
#include "adhd.h"

//#define USE_VM_MONITOR 1

#if USE_VM_MONITOR
#include "vm_debug.h"
#endif /* USE_VM_MONITOR */

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


static void vm_sysc_puts( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   FILEPTR_T ipc_offset = 0,
      bytes_read = 0;
   unsigned char cbuf = 0;

   /* TODO: Use actual printf w/ format strings. */
   ipc_offset = vm_stack_pop( task );
   bytes_read = mfat_get_dir_entry_data(
      task->file_offset,
      ipc_offset,
      &cbuf, 1,
      task->disk_id, task->part_id );
   assert( 0 < bytes_read ); /* TODO: Crash program. */
   while( 0 < cbuf ) {
      tputc( cbuf );
      ipc_offset += 1;
      bytes_read = mfat_get_dir_entry_data(
         task->file_offset,
         ipc_offset,
         &cbuf, 1,
         task->disk_id, task->part_id );
      assert( 0 < bytes_read ); /* TODO: Crash program. */
   }
}

static void vm_sysc_droot( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint16_t offset = 0;
   uint8_t disk_id = 0,
      part_id = 0;

   disk_id = vm_stack_pop( task );
   part_id = vm_stack_pop( task );

   offset = mfat_get_root_dir_offset( disk_id, part_id );

   vm_stack_dpush( task, offset );
}

static void vm_sysc_dfirst( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint16_t offset = 0;
   uint8_t disk_id = 0,
      part_id = 0;

   disk_id = vm_stack_pop( task );
   part_id = vm_stack_pop( task );
   offset = vm_stack_dpop( task );

   offset = mfat_get_dir_entry_first_offset( offset, disk_id, part_id );

   vm_stack_dpush( task, offset );
}

static void vm_sysc_dnext( TASK_PID pid ) {
}

static void vm_sysc_dname( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint16_t offset = 0;
   uint8_t disk_id = 0,
      part_id = 0;
   char* filename = NULL;

   filename = mget( pid, vm_stack_dpop( task ), 0 );
   offset = vm_stack_dpop( task );
   disk_id = vm_stack_pop( task );
   part_id = vm_stack_pop( task );

   mfat_get_dir_entry_name( filename, offset, disk_id, part_id );
}

static void vm_sysc_icmp( TASK_PID pid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   char* cmp1 = NULL,
      * cmp2 = NULL;
   STRLEN_T res = 0;
   uint8_t len = 0;
   char sep = ' ';

   sep = vm_stack_pop( task );
   len = vm_stack_pop( task );
   cmp1 = mget( pid, vm_stack_dpop( task ), 0 );
   cmp2 = mget( pid, vm_stack_dpop( task ), 0 );

   res = alpha_cmp_cc( cmp1, len, cmp2, len, sep, false, false );

   vm_stack_push( task, (uint8_t)res );
}

static void vm_instr_sysc( TASK_PID pid, uint8_t call_id ) {
   struct adhd_task* task = &(g_tasks[pid]);
   union vm_data_type data;
   unsigned char cbuf = 0;
   char* str_ptr = NULL;

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

   case VM_SYSC_MPUTS:
      data.ipc = vm_stack_dpop( task );
      str_ptr = mget( pid, data.ipc, 0 );
      while( '\0' != *str_ptr ) {
         tputc( *str_ptr );
         str_ptr++;
      }
      break;

   case VM_SYSC_PUTS:
      vm_sysc_puts( pid );
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

   case VM_SYSC_ICMP:
      vm_sysc_icmp( pid );
      break;
   }
}

static SIPC_PTR vm_instr_branch( TASK_PID pid, uint8_t instr, IPC_PTR addr ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t comp1 = 0,
      comp2 = 0;
   uint16_t dcomp1 = 0,
      dcomp2 = 0;
   IPC_PTR retval = task->ipc + 1;
   
   switch( instr ) {
   case VM_INSTR_JUMP:
      retval = addr;
      break;

   case VM_INSTR_JSNZ:
      comp1 = vm_stack_pop( task );
      if( 0 != comp1 ) {
         retval = addr;
      }
      vm_stack_push( task, comp1 );
      break;

   case VM_INSTR_JSZ:
      comp1 = vm_stack_pop( task );
      if( 0 == comp1 ) {
         retval = addr;
      }
      vm_stack_push( task, comp1 );
      break;

   case VM_INSTR_JSZD:
      dcomp1 = vm_stack_dpop( task );
      if( 0 == dcomp1 ) {
         retval = addr;
      }
      vm_stack_dpush( task, dcomp1 );
      break;

   case VM_INSTR_JSEQ:
      comp2 = vm_stack_pop( task );
      comp1 = vm_stack_pop( task );
      if( comp1 == comp2 ) {
         retval = addr;
      }
      vm_stack_push( task, comp1 );
      break;

   case VM_INSTR_JSNE:
      comp2 = vm_stack_pop( task );
      comp1 = vm_stack_pop( task );
      if( comp1 != comp2 ) {
         retval = addr;
      }
      vm_stack_push( task, comp1 );
      break;

   case VM_INSTR_JSGE:
      comp2 = vm_stack_pop( task );
      comp1 = vm_stack_pop( task );
      if( comp1 >= comp2 ) {
         retval = addr;
      }
      vm_stack_push( task, comp1 );
      break;

   case VM_INSTR_JSGED:
      dcomp2 = vm_stack_dpop( task );
      dcomp1 = vm_stack_dpop( task );
      if( dcomp1 >= dcomp2 ) {
         retval = addr;
      }
      vm_stack_dpush( task, dcomp1 );
      break;
   }

   return retval;
}

static ssize_t vm_instr_mem( TASK_PID pid, uint8_t instr, MEMLEN_T mid ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t* addr_tmp = NULL;
   MEMLEN_T offset = 0;

   switch( instr ) {
   case VM_INSTR_MALLOC:
      addr_tmp = mget( pid, mid, vm_stack_dpop( task ) );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      break;

   case VM_INSTR_MPOP:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      *addr_tmp = vm_stack_pop( task );
      break;

   case VM_INSTR_MPOPC:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      *addr_tmp = vm_stack_pop( task );
      vm_stack_push( task, *addr_tmp );
      break;

   case VM_INSTR_MPOPD:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      /* TODO: Verify memory sz is >=2 */
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      *((uint16_t*)addr_tmp) = vm_stack_dpop( task );
      break;

   case VM_INSTR_MPOPCD:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      /* TODO: Verify memory sz is >=2 */
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      *((uint16_t*)addr_tmp) = vm_stack_dpop( task );
      vm_stack_dpush( task, *((uint16_t*)addr_tmp) );
      break;

   case VM_INSTR_MPOPCO:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      offset = vm_stack_dpop( task );
      *(addr_tmp + offset) = vm_stack_pop( task );
      vm_stack_push( task, *(addr_tmp + offset) );
      break;

   case VM_INSTR_MPOPO:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      offset = vm_stack_dpop( task );
      *(addr_tmp + offset) = vm_stack_pop( task );
      break;

   case VM_INSTR_MPUSHC:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      vm_stack_push( task, *addr_tmp );
      break;

   case VM_INSTR_MPUSHCD:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      /* TODO: Verify memory sz is >=2 */
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      vm_stack_dpush( task, *((uint16_t*)addr_tmp) );
      break;

   case VM_INSTR_MPUSHCO:
      addr_tmp = mget( pid, mid, 0 );
      /* Not NULL or offset of data from NULL.*/
      assert( NULL != addr_tmp && (void*)0x4 != addr_tmp );
      offset = vm_stack_pop( task );
      vm_stack_push( task, *(addr_tmp + offset) );
      break;

   case VM_INSTR_MFREE:
      mfree( pid, mid );
      break;
   }

   return task->ipc + 1;
}

ssize_t vm_instr_execute( TASK_PID pid, uint16_t instr_full ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t instr = instr_full >> 8;
   uint8_t data = instr_full & 0xff;
   uint16_t ddata = 0;

   assert( 0 <= pid );
   assert( 0 <= task->ipc );

#if USE_VM_MONITOR
   if( g_double_instr ) {
      printf( "ipc: %d, double data: %d (0x%x), stack: %d\n", task->ipc, instr_full, instr_full, task->stack_len );
   } else {
      int j = 0, k = 0;
      while(
        -1  != vm_instr_debug[j].val &&
        ((g_double_instr && g_double_instr != vm_instr_debug[j].val) ||
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
            "ipc: %d, instr: %s (%d) (0x%x), sysc: %s (%d) (0x%x), stack: %d\n",
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
            "ipc: %d, instr: %s (%d) (0x%x), data: %d (0x%x), stack: %d\n",
            task->ipc,
            vm_instr_debug[j].name,
            instr,
            instr,
            data,
            data,
            task->stack_len );
      }
   }

   assert( g_double_instr || 0 != instr );
#endif /* USE_VM_MONITOR */

   /* Process instructions with double parameters if this is 2nd cycle. */
   if( VM_INSTR_PUSHD == g_double_instr ) {
      g_double_instr = 0;
      /* instr_full is double data, here. */
      vm_stack_dpush( task, instr_full );
      return task->ipc + 1; /* Done processing. */
   
   } else if(
      VM_INSTR_MMIN <= g_double_instr && VM_INSTR_MMAX >= g_double_instr
   ) {
      ddata = g_double_instr;
      g_double_instr = 0;
      /* instr_full is double data, here. */
      return vm_instr_mem( pid, ddata, instr_full );

   } else if(
      VM_INSTR_JMIN <= g_double_instr && VM_INSTR_JMAX >= g_double_instr
   ) {
      ddata = g_double_instr;
      g_double_instr = 0;

      /* instr_full is double data, here. */
      return vm_instr_branch( pid, ddata, instr_full );
   }

   /* Process normal instructions and 1st cycles. */
   if( VM_INSTR_PUSH == instr ) {
      vm_stack_push( task, data );

   } else if( VM_INSTR_PUSHD == instr ) {
      /* Push will happen on next execute with full number. */
      g_double_instr = VM_INSTR_PUSHD;

   } else if( VM_INSTR_SPOP == instr ) {
      vm_stack_pop( task );

   } else if( VM_INSTR_SDPOP == instr ) {
      vm_stack_dpop( task );

   } else if( VM_INSTR_SADD == instr ) {
      data = vm_stack_pop( task ) + vm_stack_pop( task );
      vm_stack_push( task, data );

   } else if( VM_INSTR_SADDD == instr ) {
      ddata = vm_stack_dpop( task ) + vm_stack_dpop( task );
      vm_stack_dpush( task, ddata );

   } else if( VM_INSTR_SYSC == instr ) {
      vm_instr_sysc( pid, data );
   
   } else if( VM_INSTR_EXIT == instr ) {
      return -1;

   } else if(
      VM_INSTR_JMIN <= instr && VM_INSTR_JMAX >= instr
   ) {
      g_double_instr = instr;
   
   } else if(
      VM_INSTR_MMIN <= instr && VM_INSTR_MMAX >= instr
   ) {
      g_double_instr = instr;
   }

   return task->ipc + 1;
}

