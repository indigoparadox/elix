
#include "code16.h"

#define VM_C
#include "vm.h"
#include "console.h"
#include "adhd.h"
#include "mem.h"

#ifdef USE_ERROR_CODES
static const char gc_mem_error[] = "EVM02\n";
#else
static const char gc_mem_error[] = "aborting; mem error\n";
#endif /* USE_ERROR_CODES */

int16_t vm_op_POP( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   int16_t dout = 0;
   if(
      ((VM_FLAG_DBL == (flags & VM_FLAG_DBL)) && 1 >= proc->stack_len) ||
      0 >= proc->stack_len
   ) {
      return VM_ERROR_STACK;
   }
   proc->stack_len--;
   dout |= (proc->stack[proc->stack_len]) & 0x00ff;
   if( VM_FLAG_DBL == (flags & VM_FLAG_DBL) ) {
      proc->stack_len--;
      dout |= (proc->stack[proc->stack_len] << 8) & 0xff00;
   }
   return dout;
}

VM_SIPC vm_op_PUSH( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   if(
      ((VM_FLAG_DBL == (flags & VM_FLAG_DBL)) &&
         VM_STACK_MAX <= proc->stack_len + 2) ||
      VM_STACK_MAX <= proc->stack_len + 1
   ) {
      return VM_ERROR_STACK;
   }
   if( VM_FLAG_DBL == (flags & VM_FLAG_DBL) ) {
      proc->stack[proc->stack_len] = (uint8_t)(((data) >> 8) & 0x00ff);
      proc->stack_len++;
   }
   proc->stack[proc->stack_len] = (uint8_t)((data) & 0x00ff);
   proc->stack_len++;

   return proc->ipc + 1;
}

VM_SIPC vm_op_NOP( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   return proc->ipc + 1;
}

VM_SIPC vm_op_MAX( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   return proc->ipc + 1;
}

VM_SIPC vm_op_SECT( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   return proc->ipc + 1;
}

VM_SIPC vm_op_JSEQ( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   int16_t comp1 = 0,
      comp2 = 0;

   comp2 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp2 ) { return comp2; }
   comp1 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp1 ) { return comp1; }
   vm_dprintf( 0, "%d vs %d", comp1, comp2 );
   if( comp1 == comp2 ) {
      return data;
   }

   /* Only pop the second comparator, so put the first back. */
   if( VM_ERROR_STACK == vm_op_PUSH( proc, flags, comp1 ) ) {
      return VM_ERROR_STACK;
   }

   return proc->ipc + 1;
}

VM_SIPC vm_op_JUMP( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   return data;
}

VM_SIPC vm_op_JSNE( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   int16_t comp1 = 0,
      comp2 = 0;

   comp2 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp2 ) { return comp2; }
   comp1 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp1 ) { return comp1; }
   vm_dprintf( 0, "%d vs %d", comp1, comp2 );
   if( comp1 != comp2 ) {
      return data;
   }

   /* Only pop the second comparator, so put the first back. */
   if( VM_ERROR_STACK == vm_op_POP( proc, flags,comp1 ) ) {
      return VM_ERROR_STACK;
   }

   return proc->ipc + 1;
}

VM_SIPC vm_op_JSGE( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   int16_t comp1 = 0,
      comp2 = 0;

   comp2 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp2 ) { return comp2; }
   comp1 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp1 ) { return comp1; }
   vm_dprintf( 0, "%d vs %d", comp1, comp2 );
   if( comp1 >= comp2 ) {
      return data;
   }

   /* Only pop the second comparator, so put the first back. */
   if( VM_ERROR_STACK == vm_op_PUSH( proc, flags, comp1 ) ) {
      return VM_ERROR_STACK;
   }

   return proc->ipc + 1;
}

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

VM_SIPC vm_op_SJUMP( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   VM_SIPC ipc_out = 0,
      i = 0;

   assert( VM_FLAG_DBL != (flags & VM_FLAG_DBL) );

   ipc_out = vm_op_POP( proc, flags, 0 );

   /* Slip the current address into the bottom of the stack. */
   if( VM_STACK_MAX <= proc->stack_len + 2 ) {
      return VM_ERROR_STACK;
   }
   proc->stack_len += 2;
   for( i = proc->stack_len - 1 ; i > 1 ; i-- ) {
      proc->stack[i] = proc->stack[i - 2];
   }

   /* We want to return to the NEXT instruction. */
   proc->ipc += 1;
   proc->stack[0] = (proc->ipc << 8) & 0x00ff;
   proc->stack[1] = proc->ipc & 0x00ff;

   return ipc_out;
}

VM_SIPC vm_op_SRET( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   VM_SIPC ipc_out = 0,
      i = 0;
      
   /* Grab jump address from the bottom of the stack. */
   if( 2 > proc->stack_len ) {
      return VM_ERROR_STACK;
   }

   ipc_out |= (proc->stack[0] & 0x00ff) << 8;
   ipc_out |= proc->stack[1] & 0x00ff;
   for( i = 0 ; i < proc->stack_len - 2 ; i++ ) {
      proc->stack[i] = proc->stack[i + 1];
   }

   return ipc_out;
}

VM_SIPC vm_op_SPOP( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   if( VM_ERROR_STACK == vm_op_POP( proc, flags, 0 ) ) {
      return VM_ERROR_STACK;
   }

   return proc->ipc + 1;
}

VM_SIPC vm_op_SADD( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   int16_t val1 = 0,
      val2 = 0;

   val1 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == val1 ) { return VM_ERROR_STACK; }
   val2 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == val2 ) { return VM_ERROR_STACK; }
   return vm_op_PUSH( proc, flags, val1 + val2 );

   return proc->ipc + 1;
}

VM_SIPC vm_op_SYSC( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   /* TODO */
   return proc->ipc + 1;
}

#if 0

VM_SIPC vm_instr_execute( TASK_PID pid, uint16_t instr_full ) {
   struct adhd_task* task = &(g_tasks[pid]);
   uint8_t instr = instr_full >> 8;
   uint8_t data = instr_full & 0xff;
   uint16_t arg = 0;

   assert( 0 <= pid );
   assert( 0 <= task->ipc );

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

#endif

