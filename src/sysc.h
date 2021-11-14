
#ifndef SYSC_H
#define SYSC_H

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
   f( MFREE,      "mfree" )

#ifdef VM_ASSM

/* === If we're being called inside vm.c === */

#define SYSC_STR_LIST( op, token ) token,

const char* gc_sysc_tokens[] = {
   SYSC_TABLE( SYSC_STR_LIST )
   "" /* Terminator for easier looping. */
};

#endif /* !VM_ASSM */

#endif /* !SYSC_H */

