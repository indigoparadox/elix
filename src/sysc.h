
#ifndef SYSC_H
#define SYSC_H

#include "etypes.h"

/**
 * | Syscall | Description                      | Args on Stack
 * |---------|----------------------------------|---------------
 * | NOOP    | Internal use only.               | None.
 * | PUTC    | Put a char on-screen.            | Char to put on-screen.
 * | PUTS    | Put a string on-screen.          |
 * | GETC    | Get char from input onto stack.  |
 * | CMP     |
 * | ICMP    |
 * | DENTRY  |
 * | DROOT   |
 * |
 */
#define SYSC_TABLE( f ) \
   f( NOOP,       "noop" ) \
   f( PUTC,       "putc" ) \
   f( PUTS,       "puts" ) \
   f( GETC,       "getc" ) \
   f( CMP,        "cmp" ) \
   f( ICMP,       "icmp" ) \
   f( DENTRY,     "dentry" ) \
   f( DROOT,      "droot" ) \
   f( DFIRST,     "dfirst" ) \
   f( DNEXT,      "dnext" ) \
   f( DNAME,      "dname" ) \
   f( MPUTS,      "mputs" ) \
   f( FLAGON,     "flagon" ) \
   f( FLAGOFF,    "flagoff" ) \
   f( LAUNCH,     "launch" ) \
   f( EXIT,       "exit" ) \
   f( MALLOC,     "malloc" ) \
   f( MFREE,      "mfree" ) \
   f( MPOP,       "mpop" ) \
   f( MPUSH,      "mpush" ) \
   f( MALLOC,     "malloc" ) \
   f( MFREE,      "mfree" ) \

#define SYSC_ERROR_MEM              -32758
#define SYSC_ERROR_DISK             -32756
#define SYSC_ERROR_UNIMPLEMENTED    -32755
#define SYSC_ERROR_EXIT             -32754

typedef VM_SIPC (*SYSC)( TASK_PID pid, uint8_t flags );

#define SYSC_PROTOTYPES( sysc, token ) \
   VM_SIPC sysc_ ## sysc ( TASK_PID pid, uint8_t flags );

SYSC_TABLE( SYSC_PROTOTYPES )

#ifdef SYSC_C

#ifndef ASSM_NO_VM

#define SYSC_CB_LIST( op, token ) sysc_ ## op,

const SYSC gc_sysc_cbs[] = {
   SYSC_TABLE( SYSC_CB_LIST )
};

#endif /* !ASSM_NO_VM */

#else

#ifndef ASSM_NO_VM
extern const SYSC gc_sysc_cbs[];
#endif /* !ASSM_NO_VM */

#endif /* SYSC_C */

#endif /* !SYSC_H */

