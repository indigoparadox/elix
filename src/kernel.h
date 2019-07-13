
#ifndef KERNEL_H
#define KERNEL_H

/*! \file kernel.h
 *  \brief General definitions and types for internal OS processes.
 */

#include "etypes.h"

#define STDIN 0

#ifndef VERSION
#define VERSION "2019.21"
#endif /* VERSION */

#define KERNEL_MID_CLI 5

/*! \addtogroup ctor_prio Constructor Priorities
 *  \brief These decide in what order constructors will be invoked at the
 *         beginning of execution.
 *  @{
 */

#define CTOR_PRIO_WDT 101
#define CTOR_PRIO_MEM 102
#define CTOR_PRIO_DISPLAY 105
#define CTOR_PRIO_UART 110

/*! @} */

/*! \brief  Main execution loop.
 *  @param argc Only on some platforms. Allows passing commands to execute in
 *              the repl from the host OS.
 *  @param argv Only on some platforms. Allows passing commands to execute in
 *              the repl from the host OS.
 */
int kmain(
#ifdef USE_EXT_CLI
int argc, char** argv
#endif /* USE_EXT_CLI */
);

#include "globals.h"

#endif /* KERNEL_H */

