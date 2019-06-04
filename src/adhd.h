
#ifndef ADHD_H
#define ADHD_H

#include <stdint.h>

#include "mem.h"

#define ADHD_TASKS_MAX 5

typedef uint8_t TASK_PID;
typedef uint8_t TASK_RETVAL;

#define RETVAL_OK 0
#define RETVAL_KILL 255

struct adhd_task;

TASK_PID adhd_add_task( TASK_RETVAL (*callback)( TASK_PID ) );
TASK_PID adhd_get_tasks_len();
TASK_RETVAL adhd_call_task( TASK_PID pid );
void adhd_kill_task( TASK_PID pid );
void adhd_wait( BITFIELD status, BITFIELD condition );

#endif /* ADHD_H */

