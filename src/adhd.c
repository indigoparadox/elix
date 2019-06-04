
#include "adhd.h"

#include <stddef.h>

struct adhd_task {
   unsigned long period;
   unsigned long elapsed;
   TASK_RETVAL (*callback)( TASK_PID );
};

static struct adhd_task g_tasks[ADHD_TASKS_MAX];

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

