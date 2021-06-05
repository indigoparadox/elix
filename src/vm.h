
#ifndef VM_H
#define VM_H

#include "etypes.h"

#define VM_INSTR_NOP    0x00
#define VM_INSTR_EXIT   0x01
#define VM_INSTR_GOTO   0x02
#define VM_INSTR_SYSC   0x03
#define VM_INSTR_SECT   0x0f
#define VM_INSTR_PUSH   0x10
#define VM_INSTR_POP    0x11
#define VM_INSTR_ADD    0x20
#define VM_INSTR_JSNZ   0x30
#define VM_INSTR_JSEQ   0x31
#define VM_INSTR_JSNE   0x32

#define VM_SYSC_PUTC    0x01
#define VM_SYSC_PRINTF  0x02
#define VM_SYSC_GETC    0x03
#define VM_SYSC_ALLOC   0x04
#define VM_SYSC_MSTORE  0x05
#define VM_SYSC_MGET    0x06
#define VM_SYSC_FREE    0x07

#define VM_SECTION_DATA 0x01
#define VM_SECTION_CPU  0x02

ssize_t vm_instr_execute( TASK_PID pid, uint16_t instr_full );

#endif /* VM_H */

