
#ifndef STRINGS_H
#define STRINGS_H

#include "alpha.h"

#ifdef STRINGS_C
#define astring_const( id, str ) const struct astring id = astring_l( str )
#else
#define astring_const( id, str ) extern struct astring id
#endif /* STRINGS_C */

astring_const( g_str_invalid, "invalid\n" );
astring_const( g_str_newline, "\n" );
astring_const( g_str_xx, "** " );
astring_const( g_str_x, "%2x " );
astring_const( g_str_frames_rcvd, "frames rcvd: %d\n" );
astring_const( g_str_responding, "responding\n" );
astring_const( g_str_no_socket, "no socket\n" );
#ifdef NET_CON_ECHO
astring_const( g_str_self_match, "self match\n" );
#endif /* NET_CON_ECHO */

#endif /* STRINGS_H */

