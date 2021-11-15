
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

#endif /* !SYSC_H */

