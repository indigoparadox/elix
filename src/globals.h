
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

/*! \addtogroup con_dev Console Device
 *  \brief (2 bytes) These globals handle which hardware device prints and
 *         receives user output/input.
 *  @{
 */

#ifndef QD_CONSOLE_IN_IDX
/*! \brief The default console input device index, if none defined in CFLAGS.
 */
#define QD_CONSOLE_IN_IDX 0
#endif /* QD_CONSOLE_IN_IDX */

#ifndef QD_CONSOLE_OUT_IDX
/*! \brief The default console output device index, if none defined in CFLAGS.
 */
#define QD_CONSOLE_OUT_IDX 0
#endif /* QD_CONSOLE_OUT_IDX */

#ifndef CONSOLE_C
extern
#endif /* !CONSOLE_C */
/*! \brief (1 byte) The currently selected console input device index.
 */
uint8_t g_console_in_dev_index
#ifdef CONSOLE_C
= QD_CONSOLE_IN_IDX
#endif /* CONSOLE_C */
;

#ifndef CONSOLE_C
extern
#endif /* !CONSOLE_C */
/*! \brief (1 byte) The currently selected console output device index.
 */
uint8_t g_console_out_dev_index
#ifdef CONSOLE_C
= QD_CONSOLE_OUT_IDX
#endif /* CONSOLE_C */
;

/*! @} */

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

#endif /* GLOBALS_H */

