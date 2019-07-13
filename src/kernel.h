
#ifndef KERNEL_H
#define KERNEL_H

/*! \file kernel.h
 *  \brief  General definitions and types for internal OS processes.
 */

#include <stdint.h>

#include "adhd.h"
#include "alpha.h"
#include "console.h"

#define STDIN 0

#ifndef VERSION
#define VERSION "2019.21"
#endif /* VERSION */

/*! \addtogroup sys_state System State
 *  \brief This system allows executing a major operation (like shutdown)
 *         based on the value of a global variable.
 *  @{
 */

#define SYSTEM_RUNNING 0
#define SYSTEM_SHUTDOWN 1

/* @} */

#define KERNEL_MID_CLI 5

#define CMD_MAX_LEN 10

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

struct api_command {
   char command[CMD_MAX_LEN]; /*!< Command entered into repl to invoke. */
   CONSOLE_CMD callback;      /*!< Callback to be executed on invocation. */
} __attribute__( (packed) );

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

#ifndef KERNEL_C
extern
#endif /* !KERNEL_C */
/*! Set this to execute a system-wide operation from the main loop.
 *  \ingroup sys_state
 */
uint8_t g_system_state
#ifdef KERNEL_C
 = SYSTEM_RUNNING
#endif /* KERNEL_C */
;

#endif /* KERNEL_H */

