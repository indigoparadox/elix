
#ifndef GLOBALS_H
#define GLOBALS_H

#include "platform.h"

/*! \file globals.h */

/*! \addtogroup sys_state System State
 *  \brief (1 byte) This system allows executing a major operation (like
 *         shutdown) based on the value of a global variable.
 *  @{
 */

#define SYSTEM_RUNNING 0
#define SYSTEM_SHUTDOWN 1

#ifndef KERNEL_C
extern
#endif /* !KERNEL_C */
/*! \brief (1 byte) Set this to execute a system-wide op from the main loop.
 */
uint8_t g_system_state
#ifdef KERNEL_C
 = SYSTEM_RUNNING
#endif /* KERNEL_C */
;

/* @} */

/*! \addtogroup con_share Console Sharing
 *  \brief (<10 bytes) These globals handle how the console subsystem is shared.
 *  @{
 */

/*! \brief The number of console commands able to be registered by apps.
 */
#define CONSOLE_CMDS_MAX 5

#ifndef CONSOLE_C
extern
#endif /* !CONSOLE_C */
/*! \brief (2 bytes) the PID of the app currently using the console.
 */
TASK_PID g_console_pid
#ifdef CONSOLE_C
= 0
#endif /* CONSOLE_C */
;

#ifndef CONSOLE_C
extern
#endif /* !CONSOLE_C */
/*! \brief (ptr bytes * CONSOLE_CMDS_MAX#) Callbacks to proc unknown commands.
 * These callbacks should be registered by their apps in the app's constructor.
 */
CONSOLE_CMD g_console_apps[CONSOLE_CMDS_MAX]
#ifdef CONSOLE_C
= { 0 }
#endif /* CONSOLE_C */
;

#ifndef CONSOLE_C
extern
#endif /* !CONSOLE_C */
uint8_t g_console_apps_top
#ifdef CONSOLE_C
= 0
#endif /* CONSOLE_C */
;

/*! @} */

#ifndef IO_C
extern
#endif /* !IO_C */
volatile uint8_t g_io_flags
#ifdef IO_C
= 0
#endif /* IO_C */
;

#endif /* GLOBALS_H */

