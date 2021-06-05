
#ifndef ETYPES_H
#define ETYPES_H

/*! \file etypes.h */

#ifdef __GNUC__

#  define struct_packed( name, def ) \
      struct name { def } __attribute__((packed))

#define const_char const char
#define const_uint8_t const uint8_t
#define const_uint16_t const uint16_t
#define const_uint32_t const uint32_t

#  else

#  define struct_packed( name, def ) \
      struct name { def }

#define const_char char
#define const_uint8_t uint8_t
#define const_uint16_t uint16_t
#define const_uint32_t uint32_t

#endif /* __GNUC__ */

#ifdef NO_STD_HEADERS

typedef unsigned char      uint8_t;
typedef char               int8_t;
typedef unsigned short int uint16_t;
typedef short int          int16_t;
typedef unsigned long int  uint32_t;
typedef long int           int32_t;

#ifndef NULL
#define NULL (void*)0x00
#endif /* NULL */

typedef unsigned char bool;
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#define size_t uint16_t
#define ssize_t int16_t

#else

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef MSP430
typedef int16_t ssize_t;
#endif /* MSP430 */

#endif /* NO_STD_HEADERS */

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

/*! \brief An ID used to identify a memory allocation block within a process.
 */
typedef uint8_t MEM_ID;

/*! \brief The header attached to a dynamic memory allocation block.
 */
struct_packed( mvar,
   uint8_t pid;      /*!< \brief Process ID of owner. */
   MEM_ID mid;       /*!< \brief Memory ID of owner. */
   MEMLEN_T sz;      /*!< \brief Bytes allocated. */
   uint8_t data[0];  /*!< \brief Dummy pointer to memory block contents. */
);

/*! @} */

/*! \addtogroup etypes_mem Types: Strings
 *  @{
 */

#define STRLEN_MAX 255

/*! \brief A type that can hold the number of characters in a practical string.
 * 
 * This type exists so that larger platforms are not necessarily constrained by
 * the tiny memory amounts of the smaller platforms.
 */
typedef MEMLEN_T STRLEN_T;

struct_packed( astring,
   struct mvar mem; /*!< \brief Piggyback off of memory block for alloc size. */
   STRLEN_T len;    /*!< \brief Length of actual string data. */
   char data[];     /*!< \brief Dummy pointer to string start. */
);

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
typedef uint16_t IPC_PTR;
typedef uint8_t TASK_RETVAL;
typedef TASK_RETVAL (*ADHD_TASK)();

/*! \brief Callback, process console cmds and decide if their app handles them.
 */
typedef TASK_RETVAL (*CONSOLE_CMD)( struct astring* );

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

struct_packed( api_command,
   char command[CMD_MAX_LEN]; /*!< Command entered into repl to invoke. */
   CONSOLE_CMD callback;      /*!< Callback to be executed on invocation. */
);

#endif /* ETYPES_H */

