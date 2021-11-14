
#ifndef VM_H
#define VM_H

#include "etypes.h"

#define VM_FLAG_DBL        0x80
#define VM_FLAG_OFFSET     0x

#ifndef VM_STACK_MAX
#define VM_STACK_MAX 12
#endif /* !VM_STACK_MAX */

struct VM_PROC {
   uint16_t ipc;
   uint8_t prev_instr;
   uint8_t stack[VM_STACK_MAX];
   uint8_t stack_len;
};

#define VM_OP_TABLE( f ) \
   f(   0,  0, NOP,     "nop" ) \
   f(   1,  0, SECT,    "sect" ) \
   f(   2,  1, SYSC,    "sysc" ) \
   f(   3,  1, PUSH,    "push" ) \
   f(   4,  0, SPOP,    "spop" ) \
   f(   5,  0, SADD,    "saddd" ) \
   f(   6,  0, SJUMP,   "sjump" ) \
   f(   7,  0, SRET,    "sret" ) \
   f(   8,  1, JUMP,    "jump" ) \
   f(   9,  1, JSEQ,    "jseq" ) \
   f(  10,  1, JSNE,    "jsne" ) \
   f(  11,  1, JSGE,    "jsge" ) \
   f(  12,  1, MPOPO,   "mpopo" ) \
   f(  13,  1, MPOP,    "mpop" ) \
   f(  14,  1, MPUSHCO, "mpushco" ) \
   f(  15,  1, MPUSHC,  "mpushc" ) \
   f(  16,  1, MALLOC,  "malloc" ) \
   f(  17,  1, MFREE,   "mfree" )

#define VM_SECTION_DATA 0x01
#define VM_SECTION_CPU  0x02

/* SIPC_PTR vm_instr_execute( TASK_PID pid, uint16_t instr_full ); */

typedef uint16_t (*VM_OP)( struct VM_PROC proc, uint8_t flags );

#define VM_OP_PROTOTYPES( idx, argc, op, token ) \
   uint16_t vm_op_ ## op ( struct VM_PROC proc, uint8_t flags );

VM_OP_TABLE( VM_OP_PROTOTYPES )

#ifdef VM_ASSM

/* === If we're being called inside vm.c === */

#define VM_OP_STR_LIST( idx, argc, op, token ) token,

const char* gc_vm_op_tokens[] = {
   VM_OP_TABLE( VM_OP_STR_LIST )
   "" /* Terminator for easier looping. */
};

#define VM_OP_IDX_LIST( idx, argc, op, token ) \
   const uint8_t VM_OP_ ## op = idx;

VM_OP_TABLE( VM_OP_IDX_LIST );

#define VM_OP_ARGC_LIST( idx, argc, op, token ) argc,

const uint8_t gc_vm_op_argcs[] = {
   VM_OP_TABLE( VM_OP_ARGC_LIST )
};

#endif /* !VM_ASSM */

#ifdef VM_C

#define VM_OP_LIST( idx, argc, op, token ) vm_op_ ## op,

#else

#endif /* VM_C */

#endif /* VM_H */

