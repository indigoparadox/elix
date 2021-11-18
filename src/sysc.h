
#ifndef SYSC_H
#define SYSC_H

#include "etypes.h"

/**
 * \addtogroup vm Virtual Machine
 * \brief
 *
 * \{
 */

/*!
 * \addtogroup sysc_ref_sect Virtual Machine Syscalls
 * \brief
 *
 * \{
 * \page sysc_ref Virtual Machine SysCall Reference
 * \tableofcontents
 * 
 * This language has not quite been finalized, so adjustments are anticipated.
 *
 * \section sysc_putc PUTC  
 * \section sysc_puts PUTS
 * \section sysc_getc GETC
 * \section sysc_cmp CMP
 * \section sysc_icmp ICMP
 * \section sysc_dentry DENTRY
 * \section sysc_droot DROOT
 * \section sysc_dfirst DFIRST
 * \section sysc_dnext DNEXT
 * \section sysc_dname DNAME
 * \section sysc_mputs MPUTS
 * \section sysc_flagon FLAGON
 * \section sysc_flagoff FLAGOFF
 * \section sysc_launch LAUNCH
 *
 * \}
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

/**
 * \addtogroup sysc_errors SysCall Errors
 * \brief Error codes that may be returned from a SYSC.
 *
 * \{
 */

/*! \brief ::VM_SIPC indicating error allocating or accessing memory. */
#define SYSC_ERROR_MEM              -32758
/*! \brief ::VM_SIPC indicating error accessing disk. */
#define SYSC_ERROR_DISK             -32756
/*! \brief ::VM_SIPC indicating call has not yet been implemented. */
#define SYSC_ERROR_UNIMPLEMENTED    -32755
/*! \brief Not technically an error. :VM_SIPC indicating normal program exit. */
#define SYSC_ERROR_EXIT             -32754

/*! \} */

/**
 * \brief Callback implementing a syscall from SYSC_TABLE.
 * \param pid Process ID of current user program.
 * \param flags (Unused) Flags modifying syscall.
 * \return New IPC user program should jump to
 *         (+4 moves forward one instruction).
 */
typedef VM_SIPC (*SYSC)( TASK_PID pid, uint8_t flags );

#ifndef SKIP_DOC

#define SYSC_PROTOTYPES( idx, sysc, token ) \
   VM_SIPC sysc_ ## sysc ( TASK_PID pid, uint8_t flags );

SYSC_TABLE( SYSC_PROTOTYPES )

#endif /* !SKIP_DOC */

#ifdef SYSC_C

#ifndef ASSM_NO_VM

#define SYSC_CB_LIST( idx, op, token ) sysc_ ## op,

const SYSC gc_sysc_cbs[] = {
   SYSC_TABLE( SYSC_CB_LIST )
};

#endif /* !ASSM_NO_VM */

#else

#ifndef ASSM_NO_VM
/*! \brief \ref sysc_ref_sect implementation callback lookup table. */
extern const SYSC gc_sysc_cbs[];
#endif /* !ASSM_NO_VM */

#endif /* SYSC_C */

/*! \} */

#endif /* !SYSC_H */

