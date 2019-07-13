
#ifndef CONSOLE_H
#define CONSOLE_H

/*! \file console.h */

#include <stdarg.h>

#include "alpha.h"

/*! \addtogroup console_output Console Output
 *  \brief These functions pipe output and input through a "general" set of
 *         devices.
 *
 *         If a message or input request are meant for user interaction via the
 *         current console devices (and not e.g. communication with a modem
 *         directly via UART), then that driver's index should be set and
 *         I/O executed with these functions.
 *  @{
 */

#ifndef CONSOLE_NEWLINE
/*! The default newline string, if none is defined in the CFLAGS.
 *  Some platforms may require e.g. '\\r' or '\\r\\n' or something else.
 */
#define CONSOLE_NEWLINE "\n"
#endif /* CONSOLE_NEWLINE */

/*! \brief This is a convenience macro for quickly outputting a string to the
 *         screen.
 */
#define tputs( str ) tprintf( "%a", str )

/*! \brief Set the device with the given index as the active console input
 *         device.
 */
#define tsetin( idx ) g_console_in_dev_index = idx

/*! \brief Set the device with the given index as the active console output
 *         device.
 */
#define tsetout( idx ) g_console_out_dev_index = idx

/*! \brief Print the given character to the active console.
 */
void tputc( char c );

/*! \brief Return the latest key code pressed, or 0 if no key has been pressed.
 *
 *  This function will signal the underlying driver to remove the returned key
 *  from the input queue. If this is not the desired behavior, use twaitc()
 *  instead.
 */
char tgetc();

/*! \brief Wait for a key input as part of a loop without blocking.
 *  
 *  This will not drain the input queue if there is input waiting.
 *
 *  @return 0 if no key has been pressed yet, or the key code otherwise.
 */
char twaitc();

/*! \brief A fairly standard implementation of printf with some adaptations for
 *         this environment.
 *
 * Tokens supported by this implementation are as follows:
 *
 * | Token  | Type               | Formatted As       | Example   |
 * |--------|--------------------|--------------------|-----------|
 * | %a     | struct astring\*   | String             | Test      |
 * | %c     | char               | Single character   | T         |
 * | %d     | int                | Decimal number     | 16        |
 * | %p     | void\*             | Hexidecimal number | 0x40      |
 * | %s     | char\*             | String             | Test      |
 * | %x     | int                | Hexidecimal number | 0xf       |
 *
 * Tokens can be modified by placing a number in front of them, indicating
 * that they should be left-padded by that number of spaces. A zero can be
 * placed in front of that number, indicating 0-padding, instead.
 *
 * e.g:
 * - %12d (a number padded to 12 chars with spaces.
 * - %6s  (a string padded to 6 chars with spaces.
 * - %08d (a number padded to 8 chars with zeroes.
 *
 */
void tprintf( const char* pattern, ... );

/*! @} */

#endif /* CONSOLE_H */

