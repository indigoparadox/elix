
#ifndef STRINGS_H
#define STRINGS_H

#include "console.h"

console_const( g_str_invalid, "invalid\n" );
console_const( g_str_newline, "\n" );
console_const( g_str_xx, "** " );
console_const( g_str_service, "service" );
#ifdef USE_NET
console_const( g_str_responding, "responding\n" );
#ifdef NET_CON_ECHO
console_const( g_str_self_match, "self match\n" );
#endif /* NET_CON_ECHO */
#endif /* USE_NET */

#endif /* STRINGS_H */

