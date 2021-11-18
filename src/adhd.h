
#ifndef ADHD_H
#define ADHD_H

/*! \file adhd.h
 *  \brief Attention Descriptors & Hypervisor Data task scheduler.
 */

#include "etypes.h"
#include "vm.h"

#include <mfat.h>

#define ADHD_MID_ENV_MAIN 1
#define ADHD_MID_ENV_SCHEDULER 2
#define ADHD_MID_ENV_CURRENT 3

#define ADHD_ERROR_NO_FILES -1

#ifndef ADHD_TASKS_MAX
/*! \brief Maximum number of tasks the system can run at once. */
#define ADHD_TASKS_MAX 3
#endif /* !ADHD_TASKS_MAX */

#ifndef ADHD_FILES_MAX
/*! \brief Maximum number of files the system can open at once. */
#define ADHD_FILES_MAX 4
#endif /* !ADHD_FILES_MAX */

/*! \brief ADHD_FILE::flags indicating a file descriptor is open and active. */
#define ADHD_FILE_FLAG_OPEN   0x01
/*! \brief ADHD_FILE::flags indicating a file descriptor is a directory. */
#define ADHD_FILE_FLAG_DIR    0x02

#define TASK_PID_INVALID -1

#define RETVAL_TOO_MANY_TASKS -2
#define RETVAL_TASK_INVALID -1

#define RETVAL_KILL 255

#define ADHD_TASK_FLAG_FOREGROUND   0x01

struct ADHD_TASK {
   uint8_t flags;
   uint8_t disk_id;
   uint8_t part_id;
   uint16_t sz;
   FILEPTR_T file_offset;
   struct VM_PROC proc;
};

struct ADHD_FILE {
   uint8_t disk_id;
   uint8_t part_id;
   uint8_t flags;
   uint32_t offset;
};

void adhd_start();

/**
 * \brief Create a new global file descriptor given the location of a file.
 * \param disk_id
 * \param part_id
 * \param offset
 * \return Index of the file object in g_files.
 */
int8_t adhd_open_file(
   uint8_t disk_id, uint8_t part_id, uint32_t offset, uint8_t flags );

void adhd_close_file( uint8_t file_id );

/* TODO: Replace this with file_id. */
/**
 * \brief Launch a new task.
 * \param disk_id The disk ID of the task executable.
 * \param part_id The disk partition ID of the task executable.
 * \param offset  The byte offset of the task executable.
 * \return
 */
TASK_PID adhd_task_launch(
   uint8_t disk_id, uint8_t part_id, FILEPTR_T offset
);

void adhd_task_execute_next( TASK_PID pid );

void adhd_task_kill( TASK_PID pid );

//void adhd_wait( BITFIELD status, BITFIELD condition );

#ifdef ADHD_C
struct ADHD_TASK g_tasks[ADHD_TASKS_MAX];
struct ADHD_FILE g_files[ADHD_FILES_MAX];
#else
/*! \brief Global tasks descriptor table. */
extern struct ADHD_TASK g_tasks[ADHD_TASKS_MAX];
/*! \brief Global file descriptor table. */
extern struct ADHD_FILE g_files[ADHD_FILES_MAX];
#endif

#endif /* ADHD_H */

