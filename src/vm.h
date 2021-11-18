
#ifndef VM_H
#define VM_H

/*! \file vm.h
 *  \brief Interpreter for running user applications.
 */

#include "etypes.h"

/**
 * \addtogroup vm Virtual Machine
 * \brief Allows user applications to execute safely in an abstract environment
 *        that's consistent in differing hardware environments.
 *
 * At its base level, the VM has a few simple ::VM_OP_TABLE codes that
 * provide program flow control and basic functionality. It also provides a
 * VM_PROC::stack on which to store data that is being worked with.
 *
 * More advanced operating system functionality (e.g. file and screen access)
 * is handled by ::SYSC_TABLE and the SYSC ::VM_OP.
 *
 * \{
 */

/*! \brief Masks off bits that may be flags to modify a VM_OP. */
#define VM_MASK_FLAGS      0xff80

/**
 * \addtogroup vm_errors Virtual Machine Errors
 * \brief Error codes that may be returned from a VM_OP.
 *
 * \{
 */

/*! \brief ::VM_SIPC indicating stack overflow. */
#define VM_ERROR_STACK           -32768
/*! \brief ::VM_SIPC indicating call has not yet been implemented. */
#define VM_ERROR_UNIMPLIMENTED   -32767

/*! \} */

#ifndef VM_STACK_MAX
/*! \brief Maximum bytes able to be stored in a process's stack. */
#define VM_STACK_MAX 12
#endif /* !VM_STACK_MAX */

/*! \brief The status of a currently executing process. */
struct VM_PROC {
   /*! \brief Byte index of current execution from start of executable file. */
   uint16_t ipc;
   /*! \brief Previous executed opcode. */
   uint8_t prev_instr;
   /*! \brief Local storage for data in the user application. */
   int16_t stack[VM_STACK_MAX];
   /*! \brief Amount of VM_PROC::stack currently in use. */
   uint8_t stack_len;
};

/**
 * | Syscall | Description                      | Args on Stack
 * |---------|----------------------------------|---------------
 * | NOOP    | Internal use only.               | None.
 * | SECT    | Not technically an opcode. Designates binary section. | None.
 */
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

/**
 * \addtogroup vm_sections Virtual Machine Executable Sections
 * \brief Sections that an executable file is divided into.
 *
 * \{
 */

/*! \brief Contains constants such as string literals. */
#define VM_SECTION_DATA 0x01
/*! \brief Contains executable code. */
#define VM_SECTION_CPU  0x02

/*! \} */

/*! \brief Signed IPC value. Negative values indicate an error. */
typedef int16_t VM_SIPC;

/**
 * \brief Callback implementing a opcode from VM_OP_TABLE.
 * \param proc
 * \param flags (Unused) Flags modifying syscall.
 * \param data
 * \return New IPC user program should jump to
 *         (+4 moves forward one instruction).
 */
typedef VM_SIPC (*VM_OP)( struct VM_PROC* proc, uint8_t flags, int16_t data );

#define VM_OP_PROTOTYPES( idx, argc, op, token ) \
   VM_SIPC vm_op_ ## op ( struct VM_PROC* proc, uint8_t flags, int16_t data );

VM_OP_TABLE( VM_OP_PROTOTYPES )

#ifdef VM_C

#define VM_OP_IDX_LIST( idx, argc, op, token ) \
   const uint8_t VM_OP_ ## op = idx;

VM_OP_TABLE( VM_OP_IDX_LIST );

#ifndef ASSM_NO_VM

#define VM_OP_CB_LIST( idx, argc, op, token ) vm_op_ ## op,

/*! \brief VM opcode callback table for VM_OP_TABLE. */
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

/*! \} */

#endif /* VM_H */

