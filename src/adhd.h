
#ifndef ADHD_H
#define ADHD_H

#define ADHD_TASKS_MAX 5

struct adhd_task;

int adhd_add_task( int (*callback)( int ) );
int adhd_get_tasks_len();
int adhd_call_task( int idx );

#endif /* ADHD_H */

