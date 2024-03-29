
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
 * At its base level, the VM has a simple \ref vm_op_ref_sect that
 * provides program flow control and basic functionality. It also provides a
 * VM_PROC::stack on which to store data that is being worked with.
 *
 * More advanced operating system functionality (e.g. file and screen access)
 * is handled by \ref sysc_ref_sect and the \ref vm_op_sysc operation.
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

/*!
 * \addtogroup vm_op_ref_sect Virtual Machine Language
 * \brief Overview of the language used by the Virtual Machine.
 *
 * \{
 * \page vm_op_ref Virtual Machine Language Reference
 * \tableofcontents
 * 
 * A brief introduction to the assembly language used to program user
 * applications for the virtual machine.
 *
 * This language has not quite been finalized, so adjustments are anticipated.
 *
 * \section vm_op_sysc SYSC
 * Calls a syscall. See \ref sysc_ref for more information on available
 * syscalls.
 * \subsection vm_op_sysc_arg Immediate Argument
 * The syscall to call.
 * 
 * \section vm_op_spush SPUSH
 * Pushes its immediate argument onto the top of the stack.
 * \subsection vm_op_spush_arg Immediate Argument
 * The value to push onto the stack.
 *
 * \section vm_op_spop SPOP
 * Pops the topmost value from the stack and discards it.
 *
 * \section vm_op_sadd SADD
 * Adds the top two numbers on the stack (removing them) and then pushes the
 * result onto the top of the stack.
 *
 * \section vm_op_sjump SJUMP
 * Pops the topmost value from the stack and jumps to it as a byte offset of
 * the user program. Places the current IPC at the bottom of the stack before
 * doing so.
 * \subsection vm_op_sjump_stack Stack
 *
 * \section vm_op_sret SRET
 * Removes the bottom-most value from the stack and jumps to it as a byte
 * offset of the user application. Designed to work with \ref vm_op_sjump.
 * \subsection vm_op_sret_stack
 *
 * \section vm_op_jump JUMP
 * Jump to the immediate argument as a byte offset of the user application.
 * \subsection vm_op_jump_arg Immediate Argument
 *
 * \section vm_op_jseq JSEQ
 * Compares the top two values on the stack, discarding the topmost, and
 * jumps to the value provided as the immediate argument as a byte offset of
 * the user application if they are equal.
 * \subsection vm_op_jseq_arg Immediate Argument
 * \subsection vm_op_jseq_stack Stack
 *
 * \section vm_op_jsne JSNE
 * Compares the top two values on the stack, discarding the topmost, and
 * jumps to the value provided as the immediate argument as a byte offset of
 * the user application if they are NOT equal.
 * \subsection vm_op_jsne_arg Immediate Argument
 * \subsection vm_op_jsne_stack Stack
 *
 * \section vm_op_jsge JSGE
 * Compares the top two values on the stack, discarding the topmost, and
 * jumps to the value provided as the immediate argument as a byte offset of
 * the user application if the second top-most is greater than or equal to
 * the discarded topmost value.
 * \subsection vm_op_jsge_arg Immediate Argument
 * \subsection vm_op_jsge_stack Stack
 *
 * \section vm_op_jsge JSGT
 * Compares the top two values on the stack, discarding the topmost, and
 * jumps to the value provided as the immediate argument as a byte offset of
 * the user application if the second top-most is greater than
 * the discarded topmost value.
 * \subsection vm_op_jsge_arg Immediate Argument
 * \subsection vm_op_jsge_stack Stack
 *
 * \section vm_op_jsge JSLT
 * Compares the top two values on the stack, discarding the topmost, and
 * jumps to the value provided as the immediate argument as a byte offset of
 * the user application if the second top-most is less than
 * the discarded topmost value.
 *
 * \subsection vm_op_jsge_arg Immediate Argument
 * \subsection vm_op_jsge_stack Stack
 * \}
 */

/*! \brief Virtual machine opcode lookup table.
 *
 * Index, Immediate Argument Count, Operation, Assembly Token
 */
#define VM_OP_TABLE( f ) \
   f(   0,  0, NOP,     "nop" ) \
   f(   1,  0, SECT,    "sect" ) \
   f(   2,  1, SYSC,    "sysc" ) \
   f(   3,  1, SPUSH,   "spush" ) \
   f(   4,  0, SPOP,    "spop" ) \
   f(   5,  0, SADD,    "sadd" ) \
   f(   6,  0, SJUMP,   "sjump" ) \
   f(   7,  0, SRET,    "sret" ) \
   f(   8,  1, JUMP,    "jump" ) \
   f(   9,  1, JSEQ,    "jseq" ) \
   f(  10,  1, JSNE,    "jsne" ) \
   f(  11,  1, JSGE,    "jsge" ) \
   f(  12,  1, JSGT,    "jsgt" ) \
   f(  13,  1, JSLT,    "jslt" ) \
   f(  14,  0, MAX,     "max" )

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

#ifndef SKIP_DOC

#define VM_OP_PROTOTYPES( idx, argc, op, token ) \
   VM_SIPC vm_op_ ## op ( struct VM_PROC* proc, uint8_t flags, int16_t data );

VM_OP_TABLE( VM_OP_PROTOTYPES )

#endif /* !SKIP_DOC */

#ifdef VM_C

#  define VM_OP_IDX_LIST( idx, argc, op, token ) \
   const uint8_t VM_OP_ ## op = idx;

VM_OP_TABLE( VM_OP_IDX_LIST );

#  ifndef ASSM_NO_VM

#     define VM_OP_CB_LIST( idx, argc, op, token ) vm_op_ ## op,

/*! \brief VM opcode callback table for VM_OP_TABLE. */
const VM_OP gc_vm_op_cbs[] = {
   VM_OP_TABLE( VM_OP_CB_LIST )
};

#  endif /* !ASSM_NO_VM */

#  if DEBUG_THRESHOLD == 0 || ASSM_NO_VM

#     define VM_OP_STR_LIST( idx, argc, op, token ) token,

const char* gc_vm_op_tokens[] = {
   VM_OP_TABLE( VM_OP_STR_LIST )
   "" /* Terminator for easier looping. */
};

#  endif /* DEBUG_THRESHOLD == 0 || ASSM_NO_VM */

#else

#  if DEBUG_THRESHOLD == 0 || ASSM_NO_VM
/*! \brief Mapping of op tokens to match with their opcodes by index. */
extern const char* gc_vm_op_tokens[];
#  endif /* DEBUG_THRESHOLD == 0 || ASSM_NO_VM */

#  ifndef ASSM_NO_VM
/*! \brief \ref vm_op_ref_sect implementation callback lookup table. */
extern const VM_OP gc_vm_op_cbs[];
#  endif /* !ASSM_NO_VM */

#  ifndef SKIP_DOC

#     define VM_OP_IDX_LIST( idx, argc, op, token ) \
   extern const uint8_t VM_OP_ ## op;

VM_OP_TABLE( VM_OP_IDX_LIST );

#  endif /* !SKIP_DOC */

#endif /* VM_C */

/*! \} */

#endif /* VM_H */

