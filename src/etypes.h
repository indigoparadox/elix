
#ifndef ETYPES_H
#define ETYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef DEBUG
#include <assert.h>
#else
#define assert( x )
#endif /* DEBUG */

/*! \addtogroup etypes_mem Types: Memory
 *  @{
 */

typedef int16_t MEMLEN_T;
typedef uint16_t BITFIELD;
typedef uint8_t MEM_ID;

struct mvar {
   uint8_t pid;      /*!< \brief Process ID of owner. */
   MEM_ID mid;       /*!< \brief Memory ID of owner. */
   MEMLEN_T sz;      /*!< \brief Bytes allocated. */
   uint8_t data[0];  /*!< \brief Dummy pointer to memory block contents. */
} __attribute__((packed));

/*! @} */

/*! \addtogroup etypes_mem Types: Strings
 *  @{
 */

typedef MEMLEN_T STRLEN_T;

struct astring {
   struct mvar mem; /*!< \brief Piggyback off of memory block for alloc size. */
   STRLEN_T len;    /*!< \brief Length of actual string data. */
   char data[];     /*!< \brief Dummy pointer to string start. */
} __attribute__( (packed) );

#define UINT8_DIGITS_MAX 8
#define UINT32_DIGITS_MAX 10
#define INT64_DIGITS_MAX 11
#define INT32_DIGITS_MAX 10
#define INT16_DIGITS_MAX 6
#define INT_DIGITS_MAX 10

#if UTOA_BITS == 16
typedef uint16_t UTOA_T; /*!< Biggest type utoa can convert to string. */
#define UTOA_DIGITS_MAX INT16_DIGITS_MAX
#elif UTOA_BITS == 32
typedef uint32_t UTOA_T; /*!< Biggest type utoa can convert to string. */
#define UTOA_DIGITS_MAX INT32_DIGITS_MAX
#else /* UTOA_BITS */
typedef uint64_t UTOA_T; /*!< Biggest type utoa can convert to string. */
#define UTOA_DIGITS_MAX INT64_DIGITS_MAX
#endif /* UTOA_BITS */

/*! @} */

/*! \addtogroup etypes_mem Types: Tasks
 *  @{
 */

/*! \brief The PID for a running task.
 */
typedef int16_t TASK_PID;
typedef uint8_t TASK_RETVAL;
typedef TASK_RETVAL (*ADHD_TASK)();

/*! \brief Callback, process console cmds and decide if their app handles them.
 */
typedef TASK_RETVAL (*CONSOLE_CMD)( const struct astring* );

#define PID_MAIN 1
#define PID_FIRST 1

#define MID_PRINTF_NUMBUF 5

/*! @} */

/*! \addtogroup io_select IO Selection System
 *  \brief Functions and variables to coordinate use of IO devices.
 * @{
 */

/*! \brief Output callback registered by drivers to print output when called.
 */
typedef void (*OUTPUT_CB)( uint8_t, char );

/*! \brief Input callback registered by drivers to receive input when called.
 */
typedef char (*INPUT_CB)( uint8_t, bool );

/*! @} */

#define CMD_MAX_LEN 10

struct api_command {
   char command[CMD_MAX_LEN]; /*!< Command entered into repl to invoke. */
   CONSOLE_CMD callback;      /*!< Callback to be executed on invocation. */
} __attribute__( (packed) );

#endif /* ETYPES_H */

