
#ifndef ADHD_H
#define ADHD_H

#include <stdint.h>

#include "mem.h"

#define ADHD_TASKS_MAX 5

#define TASK_PID uint8_t
#define TASK_RETVAL uint8_t

struct adhd_task;

TASK_PID adhd_add_task( TASK_RETVAL (*callback)( TASK_PID ) );
TASK_PID adhd_get_tasks_len();
TASK_RETVAL adhd_call_task( TASK_PID pid );
void adhd_wait( BITFIELD status, BITFIELD condition );

#endif /* ADHD_H */

