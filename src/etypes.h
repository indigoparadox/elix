
#ifndef ETYPES_H
#define ETYPES_H

#include <stdint.h>
#include <stddef.h>

/*! \addtogroup etypes_mem Types: Tasks
 *  @{
 */

typedef int16_t TASK_PID;
typedef uint8_t TASK_RETVAL;
typedef TASK_RETVAL (*ADHD_TASK)();

/*! @} */

/*! \addtogroup etypes_mem Types: Memory
 *  @{
 */

typedef int16_t MEMLEN_T;
typedef uint16_t BITFIELD;
typedef uint8_t MEM_ID;

/*! @} */

/*! \addtogroup etypes_mem Types: Strings
 *  @{
 */

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

#endif /* ETYPES_H */

