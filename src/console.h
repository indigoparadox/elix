
#ifndef CONSOLE_H
#define CONSOLE_H

/*! \file console.h */

#include <stdarg.h>

#include "alpha.h"
#include "display.h"
#include "uart.h"

#define PPCONCAT_I( a, b ) a##b
#define PPCONCAT( a, b ) PPCONCAT_I( a, b )

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
#define CONSOLE_NEWLINE CONSOLE_NEWLINE_N
#endif /* CONSOLE_NEWLINE */

/*! \brief This is a convenience macro for quickly outputting a string to the
 *         console device set at compile time.
 */
#define tputs( str ) tprintf( "%a", str )

#ifndef CONSOLE_IN

/*! \brief Wait for a key input as part of a loop without blocking.
 *  
 *  This will not drain the input queue if there is input waiting.
 *
 *  @return 0 if no key has been pressed yet, or the key code otherwise.
 */
#define twaitc() keyboard_hit()

/*! \brief Return the latest key code pressed, or 0 if no key has been pressed.
 *
 *  This function will signal the underlying driver to remove the returned key
 *  from the input queue. If this is not the desired behavior, use twaitc()
 *  instead.
 */
#define tgetc() keyboard_getc()

#define tinput_init() keyboard_init()

#else
#define twaitc() PPCONCAT( CONSOLE_IN, _hit() )
#define tgetc() PPCONCAT( CONSOLE_IN, _getc() )
#define tinput_init() PPCONCAT( CONSOLE_IN, _init() )
#endif /* CONSOLE_IN */

#ifndef CONSOLE_OUT

/*! \brief Print the given character to the console device set at compile time.
 */
#define tputc( c ) display_putc( c )

#define toutput_init() display_init()

#else
#define tputc( c ) PPCONCAT( CONSOLE_OUT, _putc( c ) )
#define toutput_init() PPCONCAT( CONSOLE_OUT, _init() )
#endif /* !CONSOLE_OUT */

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
 * | %%     | literal %          | Literal            | %         |
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

