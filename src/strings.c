
#include "alpha.h"

/* The header strings.h contains the extern defs, so we DON'T include it in
 * this file. Just all the others!
 */

const struct astring g_str_invalid = astring_l( "invalid\n" );
const struct astring g_str_newline = astring_l( "\n" );
const struct astring g_str_xx = astring_l( "**" );
const struct astring g_str_frames_rcvd = astring_l( "frames rcvd: %d\n" );
const struct astring g_str_responding = astring_l( "responding\n" );
#ifdef NET_CON_ECHO
const struct astring g_str_self_match = astring_l( "self match\n" );
#endif /* NET_CON_ECHO */

