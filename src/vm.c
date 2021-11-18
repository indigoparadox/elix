
#include "code16.h"

#include "uprintf.h"
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
   if( 0 >= proc->stack_len ) {
      return VM_ERROR_STACK;
   }
   proc->stack_len--;
   dout |= proc->stack[proc->stack_len];
   return dout;
}

VM_SIPC vm_op_PUSH( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   if( VM_STACK_MAX <= proc->stack_len + 1 ) {
      return VM_ERROR_STACK;
   }
   proc->stack[proc->stack_len] = data;
   proc->stack_len++;

   return proc->ipc + 4;
}

VM_SIPC vm_op_NOP( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   return proc->ipc + 4;
}

VM_SIPC vm_op_MAX( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   return VM_ERROR_UNIMPLIMENTED;
}

VM_SIPC vm_op_SECT( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   return proc->ipc + 2;
}

VM_SIPC vm_op_JSEQ( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   int16_t comp1 = 0,
      comp2 = 0,
      ipc_out = proc->ipc + 4;

   comp2 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp2 ) { return comp2; }
   comp1 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp1 ) { return comp1; }
   elix_dprintf( 0, "%d vs %d", comp1, comp2 );
   if( comp1 == comp2 ) {
      ipc_out = data;
   }

   /* Only pop the second comparator, so put the first back. */
   if( VM_ERROR_STACK == vm_op_PUSH( proc, flags, comp1 ) ) {
      return VM_ERROR_STACK;
   }

   return ipc_out;
}

VM_SIPC vm_op_JUMP( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   return data;
}

VM_SIPC vm_op_JSNE( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   int16_t comp1 = 0,
      comp2 = 0,
      ipc_out = proc->ipc + 4;

   comp2 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp2 ) { return comp2; }
   comp1 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp1 ) { return comp1; }
   elix_dprintf( 0, "%d vs %d", comp1, comp2 );
   if( comp1 != comp2 ) {
      ipc_out = data;
   }

   /* Only pop the second comparator, so put the first back. */
   if( VM_ERROR_STACK == vm_op_PUSH( proc, flags, comp1 ) ) {
      return VM_ERROR_STACK;
   }

   return ipc_out;
}

VM_SIPC vm_op_JSGE( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   int16_t comp1 = 0,
      comp2 = 0,
      ipc_out = proc->ipc + 4;

   comp2 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp2 ) { return comp2; }
   comp1 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == comp1 ) { return comp1; }
   elix_dprintf( 0, "%d vs %d", comp1, comp2 );
   if( comp1 >= comp2 ) {
      return data;
   }

   /* Only pop the second comparator, so put the first back. */
   if( VM_ERROR_STACK == vm_op_PUSH( proc, flags, comp1 ) ) {
      return VM_ERROR_STACK;
   }

   return proc->ipc + 4;
}

VM_SIPC vm_op_SJUMP( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   VM_SIPC ipc_out = 0,
      i = 0;

   ipc_out = vm_op_POP( proc, flags, 0 );

   /* Slip the current address into the bottom of the stack. */
   if( VM_STACK_MAX <= proc->stack_len + 1 ) {
      return VM_ERROR_STACK;
   }
   for( i = proc->stack_len ; i > 0 ; i-- ) {
      proc->stack[i] = proc->stack[i - 1];
   }
   proc->stack_len += 1;

   /* We want to return to the NEXT instruction. */
   proc->ipc += 4;
   proc->stack[0] = proc->ipc;

   return ipc_out;
}

VM_SIPC vm_op_SRET( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   VM_SIPC ipc_out = 0,
      i = 0;
      
   /* Grab jump address from the bottom of the stack. */
   if( 1 > proc->stack_len ) {
      return VM_ERROR_STACK;
   }

   ipc_out = proc->stack[0];
   proc->stack_len--;
   for( i = 0 ; i < proc->stack_len ; i++ ) {
      proc->stack[i] = proc->stack[i + 1];
   }

   return ipc_out;
}

VM_SIPC vm_op_SPOP( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   if( VM_ERROR_STACK == vm_op_POP( proc, flags, 0 ) ) {
      return VM_ERROR_STACK;
   }

   return proc->ipc + 4;
}

VM_SIPC vm_op_SADD( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   int16_t val1 = 0,
      val2 = 0;

   val1 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == val1 ) { return VM_ERROR_STACK; }
   val2 = vm_op_POP( proc, flags, 0 );
   if( VM_ERROR_STACK == val2 ) { return VM_ERROR_STACK; }
   return vm_op_PUSH( proc, flags, val1 + val2 );

   return proc->ipc + 4;
}

VM_SIPC vm_op_SYSC( struct VM_PROC* proc, uint8_t flags, int16_t data ) {
   /* This should be handled by the implementation. */
   assert( 1 == 0 );
   return VM_ERROR_UNIMPLIMENTED;
}

