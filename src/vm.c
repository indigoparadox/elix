
#include "code16.h"

#include "vm.h"
#include "console.h"
#include "adhd.h"
#include "mem.h"

//#define USE_VM_MONITOR 1

#define vm_stack_dpush( task, data ) \
   (vm_stack_push( task, (uint8_t)(((data) >> 8) & 0x00ff) ) + \
   vm_stack_push( task, (uint8_t)((data) & 0x00ff) ))

#if USE_VM_MONITOR
#include "vm_debug.h"
#endif /* USE_VM_MONITOR */

#ifdef USE_ERROR_CODES
static const char gc_stack_error[] = "EVM01\n";
static const char gc_mem_error[] = "EVM02\n";
static const char gc_stream_error[] = "EVM03\n";
#else
static const char gc_stack_error[] = "aborting; stack error\n";
static const char gc_mem_error[] = "aborting; mem error\n";
static const char gc_stream_error[] = "aborting; stream error\n";
#endif /* USE_ERROR_CODES */

static int16_t vm_stack_pop( struct adhd_task* task ) {
   if( 0 >= task->stack_len ) {
      tprintf( "%s", gc_stack_error );
      return -1;
   }
   task->stack_len--;
   return task->stack[task->stack_len];
}

static int32_t vm_stack_dpop( struct adhd_task* task ) {
   uint16_t dout = 0;
   if( 1 >= task->stack_len ) {
      tprintf( "%s", gc_stack_error );
      return -1;
   }
   task->stack_len--;
   dout |= (task->stack[task->stack_len]) & 0x00ff;
   task->stack_len--;
   dout |= (task->stack[task->stack_len] << 8) & 0xff00;
   return dout;
}

static int8_t vm_stack_push( struct adhd_task* task, uint8_t data ) {
   if( VM_STACK_MAX <= task->stack_len + 1 ) {
      tprintf( "%s", gc_stack_error );
      return -1;
   }
   task->stack[task->stack_len] = data;
   task->stack_len++;
   return 0;
}

#if 0
static int8_t vm_stack_dpush( struct adhd_task* task, uint16_t data ) {
   if( VM_STACK_MAX <= task->stack_len + 2 ) {
      tprintf( "%s", gc_stack_error );
      return -1;
   }
   task->stack[task->stack_len] = (uint8_t)((data >> 8) & 0x00ff);
   task->stack_len++;
   task->stack[task->stack_len] = (uint8_t)(data & 0x00ff);
   task->stack_len++;
   return 0;
}
#endif

static SIPC_PTR vm_instr_branch( TASK_PID pid, uint8_t instr, IPC_PTR addr ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int32_t comp1 = 0,
      comp2 = 0;
   IPC_PTR retval = task->ipc + 1;
   
   switch( instr ) {
   case VM_INSTR_JUMP:
      return addr;

   case VM_INSTR_JSEQ:
      comp2 = vm_stack_pop( task );
      if( 0 > comp2 ) { return -1; }
      comp1 = vm_stack_pop( task );
      if( 0 > comp1 ) { return -1; }
#if USE_VM_MONITOR
      printf( "%d vs %d\n", comp1, comp2 );
#endif /* USE_VM_MONITOR */
      if( comp1 == comp2 ) {
         retval = addr;
      }
      if( 0 > vm_stack_push( task, comp1 ) ) {
         return -1;
      }
      break;

   case VM_INSTR_JSED:
      comp2 = vm_stack_dpop( task );
      if( 0 > comp2 ) { return -1; }
      comp1 = vm_stack_dpop( task );
      if( 0 > comp1 ) { return -1; }
#if USE_VM_MONITOR
      printf( "%d vs %d\n", comp1, comp2 );
#endif /* USE_VM_MONITOR */
      if( comp1 == comp2 ) {
         retval = addr;
      }
      if( 0 > vm_stack_dpush( task, comp1 ) ) {
         return -1;
      }
      break;

   case VM_INSTR_JSNED:
      comp2 = vm_stack_dpop( task );
      if( 0 > comp2 ) { return -1; }
      comp1 = vm_stack_dpop( task );
      if( 0 > comp1 ) { return -1; }
#if USE_VM_MONITOR
      printf( "%d != %d?\n", comp1, comp2 );
#endif /* USE_VM_MONITOR */
      if( comp1 != comp2 ) {
         retval = addr;
#if USE_VM_MONITOR
         printf( "jumping!\n" );
#endif /* USE_VM_MONITOR */
      }
      if( 0 > vm_stack_push( task, comp1 ) ) {
         return -1;
      }
      break;

   case VM_INSTR_JSNE:
      comp2 = vm_stack_pop( task );
      if( 0 > comp2 ) { return -1; }
      comp1 = vm_stack_pop( task );
      if( 0 > comp1 ) { return -1; }
#if USE_VM_MONITOR
      printf( "%d (%c) != %d (%c)?\n", comp1, comp1, comp2, comp2 );
#endif /* USE_VM_MONITOR */
      if( comp1 != comp2 ) {
         retval = addr;
#if USE_VM_MONITOR
         printf( "jumping!\n" );
#endif /* USE_VM_MONITOR */
      }
      if( 0 > vm_stack_push( task, comp1 ) ) {
         return -1;
      }
      break;

   case VM_INSTR_JSGED:
      comp2 = vm_stack_dpop( task );
      if( 0 > comp2 ) { return -1; }
      comp1 = vm_stack_dpop( task );
      if( 0 > comp1 ) { return -1; }
      if( comp1 >= comp2 ) {
         retval = addr;
      }
      if( 0 > vm_stack_dpush( task, comp1 ) ) {
         return -1;
      }
      break;
   }

   return retval;
}

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

static SIPC_PTR vm_instr_stack( TASK_PID pid, uint8_t instr ) {
   struct adhd_task* task = &(g_tasks[pid]);
   int32_t data1 = 0,
      data2 = 0;
   SIPC_PTR retval = task->ipc + 1;

   switch( instr ) {
   case VM_INSTR_SJUMP:
      data1 = vm_stack_dpop( task );
      /* Slip the current address into the bottom of the stack. */
      if( VM_STACK_MAX <= task->stack_len + 2 ) {
         tprintf( "%s\n", gc_stack_error );
         return -1;
      }
      task->stack_len += 2;
      for( data2 = task->stack_len - 1 ; data2 > 1 ; data2-- ) {
         task->stack[data2] = task->stack[data2 - 2];
      }
      task->ipc += 1; /* We want to return to the NEXT instruction. */
      task->stack[0] = (task->ipc << 8) & 0x00ff;
      task->stack[1] = task->ipc & 0x00ff;
      return data1;

   case VM_INSTR_SRET:
      /* Grab jump address from the bottom of the stack. */
      if( 2 > task->stack_len ) {
         tprintf( "%s\n", gc_stack_error );
         return -1;
      }
      data1 |= (task->stack[0] & 0x00ff) << 8;
      data1 |= task->stack[1] & 0x00ff;
      for( data2 = 0 ; data2 < task->stack_len - 2 ; data2++ ) {
         task->stack[data2] = task->stack[data2 + 1];
      }
      return data1;

   case VM_INSTR_SPOP:
      if( 0 > vm_stack_pop( task ) ) {
         return -1;
      }
      break;

   case VM_INSTR_SADDD:
      data1 = vm_stack_dpop( task );
      if( 0 > data1 ) { return -1; }
      data2 = vm_stack_dpop( task );
      if( 0 > data2 ) { return -1; }
      if( 0 > vm_stack_dpush( task, data1 + data2 ) ) {
         return -1;
      }
      break;
   }

   return retval;
}

SIPC_PTR vm_instr_execute( TASK_PID pid, uint16_t instr_full ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t instr = instr_full >> 8;
   uint8_t data = instr_full & 0xff;
   uint16_t ddata = 0;

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
      if( 0 > vm_stack_dpush( task, instr_full ) ) {
         return -1;
      }
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
      if( 0 > vm_stack_push( task, data ) ) {
         return -1;
      }

   } else if( VM_INSTR_PUSHD == instr ) {
      /* Push will happen on next execute with full number. */
      task->prev_instr = VM_INSTR_PUSHD;

   } else if( VM_INSTR_SJUMP == instr || VM_INSTR_SRET == instr ) {
      return vm_instr_stack( pid, instr );

   } else if( VM_INSTR_SPOP == instr ) {
      return vm_instr_stack( pid, instr );

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

