
#ifndef STRINGS_H
#define STRINGS_H

#include "console.h"

console_const( g_str_invalid, "invalid\n" );
console_const( g_str_newline, "\n" );
console_const( g_str_xx, "** " );
console_const( g_str_x, "%2x " );
console_const( g_str_frames_rcvd, "frames rcvd: %d\n" );
console_const( g_str_responding, "responding\n" );
console_const( g_str_no_socket, "no socket\n" );
#ifdef NET_CON_ECHO
console_const( g_str_self_match, "self match\n" );
#endif /* NET_CON_ECHO */

#endif /* STRINGS_H */

