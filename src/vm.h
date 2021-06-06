
#ifndef VM_H
#define VM_H

#include "etypes.h"

#define VM_INSTR_NOP       0x00
#define VM_INSTR_EXIT      0x01
#define VM_INSTR_GOTO      0x02
#define VM_INSTR_SYSC      0x03
#define VM_INSTR_PUSH      0x04
#define VM_INSTR_SECT      0x0f
#define VM_INSTR_SMIN      0x10 /* DUMMY */
#define VM_INSTR_SPOP      0x10
#define VM_INSTR_SADD      0x11
#define VM_INSTR_SMAX      0x1f /* DUMMY */
#define VM_INSTR_JMIN      0x30 /* DUMMY */
#define VM_INSTR_JSNZ      0x30
#define VM_INSTR_JSEQ      0x31
#define VM_INSTR_JSNE      0x32
#define VM_INSTR_JSGE      0x33
#define VM_INSTR_JMAX      0x3f /* DUMMY */
#define VM_INSTR_MMIN      0x40 /* DUMMY */
#define VM_INSTR_MALLOC    0x40
#define VM_INSTR_MPOP      0x41 /* Pop TO memory. */
#define VM_INSTR_MPOPC     0x42 /* Pop copy TO memory. */
#define VM_INSTR_MPOPCO    0x43 /* Pop copy TO memory PLUS offset. */
#define VM_INSTR_MPUSHC    0x44 /* Push copy FROM memory. */
#define VM_INSTR_MPUSHCO   0x45 /* Push copy FROM memory PLUS offset. */
#define VM_INSTR_MFREE     0x46
#define VM_INSTR_MMAX      0x4f /* DUMMY */

#define VM_SYSC_PUTC    0x01
#define VM_SYSC_PRINTF  0x02
#define VM_SYSC_GETC    0x03

#define VM_SECTION_DATA 0x01
#define VM_SECTION_CPU  0x02

ssize_t vm_instr_execute( TASK_PID pid, uint16_t instr_full );

#endif /* VM_H */

