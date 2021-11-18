
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
   f(  0, NOOP,      "noop" ) \
   f(  1, PUTC,      "putc" ) \
   f(  2, PUTS,      "puts" ) \
   f(  3, GETC,      "getc" ) \
   f(  4, CMP,       "cmp" ) \
   f(  5, ICMP,      "icmp" ) \
   f(  6, DENTRY,    "dentry" ) \
   f(  7, DROOT,     "droot" ) \
   f(  8, DFIRST,    "dfirst" ) \
   f(  9, DNEXT,     "dnext" ) \
   f( 10, DNAME,     "dname" ) \
   f( 11, MPUTS,     "mputs" ) \
   f( 12, FLAGON,    "flagon" ) \
   f( 13, FLAGOFF,   "flagoff" ) \
   f( 14, LAUNCH,    "launch" ) \
   f( 15, EXIT,      "exit" ) \
   f( 16, MALLOC,    "malloc" ) \
   f( 17, MFREE,     "mfree" ) \
   f( 18, MPOP,      "mpop" ) \
   f( 19, MPUSH,     "mpush" ) \
   f( 20, MALLOC,    "malloc" ) \
   f( 21, MFREE,     "mfree" )

/*! \brief ::VM_SIPC indicating error allocating or accessing memory. */
#define SYSC_ERROR_MEM              -32758
/*! \brief ::VM_SIPC indicating error accessing disk. */
#define SYSC_ERROR_DISK             -32756
/*! \brief ::VM_SIPC indicating call has not yet been implemented. */
#define SYSC_ERROR_UNIMPLEMENTED    -32755
/*! \brief Not technically an error. :VM_SIPC indicating normal program exit. */
#define SYSC_ERROR_EXIT             -32754

/**
 * \brief Callback implementing a syscall from SYSC_TABLE.
 * \param pid Process ID of current user program.
 * \param flags (Unused) Flags modifying syscall.
 * \return New IPC user program should jump to
 *         (+4 moves forward one instruction).
 */
typedef VM_SIPC (*SYSC)( TASK_PID pid, uint8_t flags );

#define SYSC_PROTOTYPES( idx, sysc, token ) \
   VM_SIPC sysc_ ## sysc ( TASK_PID pid, uint8_t flags );

SYSC_TABLE( SYSC_PROTOTYPES )

#ifdef SYSC_C

#ifndef ASSM_NO_VM

#define SYSC_CB_LIST( idx, op, token ) sysc_ ## op,

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

