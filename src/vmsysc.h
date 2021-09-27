
#ifndef VMSYSC_H
#define VMSYSC_H

#include <mvm.h>

#define VM_SYSC_PUTC    0x01
#define VM_SYSC_PUTS    0x02
#define VM_SYSC_GETC    0x03
#define VM_SYSC_CMP     0x04
#define VM_SYSC_ICMP    0x05
#define VM_SYSC_DENTRY  0x06
#define VM_SYSC_DROOT   0x07
#define VM_SYSC_DFIRST  0x08
#define VM_SYSC_DNEXT   0x09
#define VM_SYSC_DNAME   0x0a
#define VM_SYSC_MPUTS   0x0b
#define VM_SYSC_FLAGON  0x0c
#define VM_SYSC_FLAGOFF 0x0d
#define VM_SYSC_LAUNCH  0x0e
#define VM_SYSC_EXIT    0x0f

SIPC_PTR vm_instr_sysc( TASK_PID pid, struct mvm_state*, uint8_t call_id );

#endif /* VMSYSC_H */

