
#include "adhd.h"

#include "mem.h"

#include <stddef.h>

struct adhd_task {
   unsigned long period;
   unsigned long elapsed;
   TASK_RETVAL (*callback)( TASK_PID );
};

static struct adhd_task g_tasks[ADHD_TASKS_MAX];
static TASK_PID g_tasks_len = 0;

TASK_PID adhd_add_task( TASK_RETVAL (*callback)( TASK_PID ) ) {
   struct adhd_task* task = NULL;

   if( ADHD_TASKS_MAX <= g_tasks_len + 1 ) {
      /* Too many tasks already! */
      return -1;
   }

   mzero( &(g_tasks[g_tasks_len]), sizeof( struct adhd_task ) );
   task = &(g_tasks[g_tasks_len]);
   task->callback = callback;

   //mcopy( &(g_tasks[tasks_len]), task, sizeof( struct adhd_task ) );

   /* Increment task count, return new task index. */
   g_tasks_len++;
   return g_tasks_len - 1;
}

TASK_PID adhd_get_tasks_len() {
   return g_tasks_len;
}

TASK_RETVAL adhd_call_task( TASK_PID pid ) {
   if( 0 > pid || pid >= g_tasks_len ) {
      /* Invalid task index. */
      return -1;
   }
   return g_tasks[pid].callback( pid );
}

void adhd_wait( BITFIELD status, BITFIELD condition ) {
   //while( test_status( status ) != condition ) {
      /* TODO: Keep calling handlers or the status may never go away. */
      /* TODO: Otherwise, just wait for IRQ. */
   //}
}

