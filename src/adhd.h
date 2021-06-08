
#ifndef ADHD_H
#define ADHD_H

#include "etypes.h"
#include "vm.h"

#include <mfat.h>

#define ADHD_MID_ENV_MAIN 1
#define ADHD_MID_ENV_SCHEDULER 2
#define ADHD_MID_ENV_CURRENT 3

#define ADHD_TASKS_MAX 4
#define ADHD_STACK_MAX 12

#define TASK_PID_INVALID -1

#define RETVAL_TOO_MANY_TASKS -2
#define RETVAL_TASK_INVALID -1

#define RETVAL_KILL 255

#define ADHD_TASK_FLAG_FOREGROUND   0x01
#define ADHD_TASK_FLAG_DEREF        0x02 /* Next memory instruction will deref. */

struct adhd_task {
   uint16_t ipc;
   uint8_t flags;
   uint8_t disk_id;
   uint8_t part_id;
   uint8_t prev_instr;
   uint16_t sz;
   FILEPTR_T file_offset;
   uint8_t stack[ADHD_STACK_MAX];
   uint8_t stack_len;
};

void adhd_start();

/*!  \brief Launch a new task.
 *   @param disk_id The disk ID of the task executable.
 *   @param part_id The disk partition ID of the task executable.
 *   @param offset  The byte offset of the task executable.
 */
TASK_PID adhd_task_launch(
   uint8_t disk_id, uint8_t part_id, FILEPTR_T offset
);

void adhd_task_execute_next( TASK_PID pid );

void adhd_task_kill( TASK_PID pid );

//void adhd_wait( BITFIELD status, BITFIELD condition );

#ifdef ADHD_C
struct adhd_task g_tasks[ADHD_TASKS_MAX];
#else
extern struct adhd_task g_tasks[ADHD_TASKS_MAX];
#endif

#endif /* ADHD_H */

