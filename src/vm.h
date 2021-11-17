
#ifndef VM_H
#define VM_H

/*! \file vm.h
 *  \brief Interpreter for running user applications.
 */

/* TODO: Flatten this out. All instructions should be 16 bit. */

#include "etypes.h"

#ifndef VM_DEBUG_THRESHOLD
#define VM_DEBUG_THRESHOLD 1
#endif /* !VM_DEBUG_THRESHOLD */

#define VM_MASK_FLAGS      0xff80

/*! \brief ::VM_SIPC indicating stack overflow. */
#define VM_ERROR_STACK           -32768
#define VM_ERROR_UNIMPLIMENTED   -32767

#ifndef VM_STACK_MAX
/*! \brief Maximum bytes able to be stored in a process's stack. */
#define VM_STACK_MAX 12
#endif /* !VM_STACK_MAX */

typedef uint8_t VM_OPCODE;

/*! \brief The status of a currently executing process. */
struct VM_PROC {
   /*! \brief Byte index of current execution from start of executable file. */
   uint16_t ipc;
   /*! \brief Previous executed opcode. */
   uint8_t prev_instr;
   int16_t stack[VM_STACK_MAX];
   uint8_t stack_len;
};

#define VM_OP_TABLE( f ) \
   f(   0,  0, NOP,     "nop" ) \
   f(   1,  0, SECT,    "sect" ) \
   f(   2,  1, SYSC,    "sysc" ) \
   f(   3,  1, PUSH,    "push" ) \
   f(   4,  0, SPOP,    "spop" ) \
   f(   5,  0, SADD,    "sadd" ) \
   f(   6,  0, SJUMP,   "sjump" ) \
   f(   7,  0, SRET,    "sret" ) \
   f(   8,  1, JUMP,    "jump" ) \
   f(   9,  1, JSEQ,    "jseq" ) \
   f(  10,  1, JSNE,    "jsne" ) \
   f(  11,  1, JSGE,    "jsge" ) \
   f(  12,  0, MAX,     "max" )

#define VM_SECTION_DATA 0x01
#define VM_SECTION_CPU  0x02

/*! \brief Signed IPC value. Negative values indicate an error. */
typedef int16_t VM_SIPC;

/**
 * \brief Opcode handler callback.
 * \param proc
 * \param flags
 * \param data
 * \return New IPC value after this instruction is executed.
 */
typedef VM_SIPC (*VM_OP)( struct VM_PROC* proc, uint8_t flags, int16_t data );

#define VM_OP_PROTOTYPES( idx, argc, op, token ) \
   VM_SIPC vm_op_ ## op ( struct VM_PROC* proc, uint8_t flags, int16_t data );

VM_OP_TABLE( VM_OP_PROTOTYPES )

#define vm_dprintf( lvl, ... ) \
   if( lvl >= VM_DEBUG_THRESHOLD ) { \
      tprintf( "(%d) " __FILE__ ": %d: ", lvl, __LINE__ ); \
      tprintf( __VA_ARGS__ ); \
      tprintf( "\n" ); \
   }

#define vm_eprintf( ... ) \
   tprintf( "(E) " __FILE__ ": %d: ", __LINE__ ); \
   tprintf( __VA_ARGS__ ); \
   tprintf( "\n" ); \

#ifdef VM_C

#define VM_OP_IDX_LIST( idx, argc, op, token ) \
   const uint8_t VM_OP_ ## op = idx;

VM_OP_TABLE( VM_OP_IDX_LIST );

#ifndef ASSM_NO_VM

#define VM_OP_CB_LIST( idx, argc, op, token ) vm_op_ ## op,

const VM_OP gc_vm_op_cbs[] = {
   VM_OP_TABLE( VM_OP_CB_LIST )
};

#endif /* !ASSM_NO_VM */

#else

#ifndef ASSM_NO_VM
extern const VM_OP gc_vm_op_cbs[];
#endif /* !ASSM_NO_VM */

#define VM_OP_IDX_LIST( idx, argc, op, token ) \
   extern const uint8_t VM_OP_ ## op;

VM_OP_TABLE( VM_OP_IDX_LIST );

#endif /* VM_C */

#endif /* VM_H */

