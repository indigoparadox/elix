
#include "adhd.h"

#include "mem.h"

#include <stddef.h>

struct adhd_task {
   unsigned long period;
   unsigned long elapsed;
   int (*callback)( int );
};

static struct adhd_task g_tasks[ADHD_TASKS_MAX];
static int g_tasks_len = 0;

int adhd_add_task( int (*callback)( int ) ) {
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

int adhd_get_tasks_len() {
   return g_tasks_len;
}

int adhd_call_task( int idx ) {
   if( 0 > idx || idx >= g_tasks_len ) {
      /* Invalid task index. */
      return -1;
   }
   return g_tasks[idx].callback( idx );
}

