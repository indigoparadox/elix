
#define ADHD_C
#include "adhd.h"

#include <stddef.h>

/*
static struct adhd_task g_tasks[ADHD_TASKS_MAX];
jmp_buf g_env_adhd;
jmp_buf g_env_main;
*/

static struct adhd_task* g_head_env = NULL;
static struct adhd_task* g_curr_env = NULL;
static uint8_t g_next_pid = ADHD_PID_FIRST;

#ifdef SCHEDULE_COOP

void adhd_step() {
   /* Setup the scheduler target and go back to main if successful. */
   if( !setjmp( g_sched_env->env ) ) {
      longjmp( g_main_env->env, 1 );
   }

   /* Scheduler target starts here. */

   /* Jump to the next task. */
   g_curr_env = g_curr_env->next;
   longjmp( g_curr_env->env, 1 );
}

struct adhd_task* adhd_new_task() {
   struct adhd_task* task = NULL;

   /* Setup the task struct and PID. */
   task = mget( ADHD_PID_MAIN, g_next_pid, sizeof( struct adhd_task ) );
   if( NULL == task ) {
      goto cleanup;
   }

   task->pid = g_next_pid;
   g_next_pid++;

   if( NULL == g_head_env ) {
      /* Start the ring with this task. */
      g_head_env = task;
      task->next = g_head_env;
      g_curr_env = g_head_env;
   } else {
      /* Insert the task into the ring. */
      task->next = g_curr_env->next;
      g_curr_env->next = task;
   }

cleanup:
   return task;
}

#else

TASK_PID adhd_add_task( TASK_RETVAL (*callback)( TASK_PID ) ) {
   struct adhd_task* task = NULL;
   TASK_PID pid_iter = 0;

   while( NULL != g_tasks[pid_iter].callback && ADHD_TASKS_MAX > pid_iter ) {
      pid_iter++;
   }
   if( ADHD_TASKS_MAX <= pid_iter ) {
      /* Too many tasks already! */
      return -1;
   }

   mzero( &(g_tasks[pid_iter]), sizeof( struct adhd_task ) );
   task = &(g_tasks[pid_iter]);
   task->callback = callback;

   /* Return new task index. */
   return pid_iter;
}

TASK_RETVAL adhd_call_task( TASK_PID pid ) {
   if( 0 > pid || pid >= ADHD_TASKS_MAX || NULL == g_tasks[pid].callback ) {
      /* Invalid task index. */
      return 0;
   }
   return g_tasks[pid].callback( pid );
}
#endif /* SCHEDULE_COOP */

#if 0
void adhd_kill_task( TASK_PID pid ) {
   if( 0 > pid || pid >= ADHD_TASKS_MAX || NULL == g_tasks[pid].callback ) {
      /* Invalid task index. */
      return;
   }
   
   g_tasks[pid].callback = NULL;
}

void adhd_wait( BITFIELD status, BITFIELD condition ) {
   //while( test_status( status ) != condition ) {
      /* TODO: Keep calling handlers or the status may never go away. */
      /* TODO: Otherwise, just wait for IRQ. */
   //}
}
#endif

