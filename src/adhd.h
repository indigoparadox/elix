
#ifndef ADHD_H
#define ADHD_H

/*! \file adhd.h
 *  \brief Attention Descriptors & Hypervisor Data task scheduler.
 */

#include "etypes.h"
#include "vm.h"

#include <mfat.h>

/**
 * \addtogroup adhd_files ADHD Files
 * \brief An abstraction layer designed to make working with files simpler
 *        while being lightweight.
 * \{
 */

/*! \brief No file structures are available in g_files to hold a new file. */
#define ADHD_ERROR_NO_FILES_AVAIL -1
/*! \brief The specified file was not found. */
#define ADHD_ERROR_FILE_NOT_FOUND -2

#ifndef ADHD_FILES_MAX
/**
 * \brief Maximum number of files the system can open at once.
 *
 * This can be overridden by the build system, though a default is provided.
 */
#define ADHD_FILES_MAX 4
#endif /* !ADHD_FILES_MAX */

/**
 * \addtogroup adhd_files_flags ADHD Files Flags
 * \brief Flags that can be applied to ADHD_FILE::flags.
 * \{
 */

/*! \brief ADHD_FILE::flags indicating a file descriptor is open and active. */
#define ADHD_FILE_FLAG_OPEN   0x01
/*! \brief ADHD_FILE::flags indicating a file descriptor is a directory. */
#define ADHD_FILE_FLAG_DIR    0x02
/*!
 * \brief ADHD_FILE::flags indicating a file descriptor is a running executable.
 */
#define ADHD_FILE_FLAG_EXEC   0x04

/** \} **/

/**
 * \brief File descriptor holding info on an open file.
 */
struct ADHD_FILE {
   /*! \brief Disk ID of the file. */
   uint8_t disk_id;
   /*! \brief Partition ID of the file. */
   uint8_t part_id;
   /*! \brief Bitfield indicating special status this file may have. */
   uint8_t flags;
   /*! \brief Offset of the file's FAT entry in bytes. */
   uint32_t offset;
   /*! \brief Size of the file on disk in bytes. */
   uint32_t sz;
};

#ifdef ADHD_C
struct ADHD_FILE g_files[ADHD_FILES_MAX];
#else
/*! \brief Global file descriptor table. */
extern struct ADHD_FILE g_files[ADHD_FILES_MAX];
#endif

/**
 * \brief Create a new global file descriptor given the location of a file.
 * \param disk_id Disk ID of the file to open.
 * \param part_id Partition ID of the file to open.
 * \param offset Offset of the file's FAT entry in bytes.
 * \param flags
 * \return Index of the file object in g_files.
 *
 * Files should be closed with adhd_file_close() when done with in order to
 * free up file descriptors for other uses.
 */
int8_t adhd_file_open(
   uint8_t disk_id, uint8_t part_id, uint32_t offset, uint8_t flags );

/**
 * \brief Close the file with the given file descriptor.
 * \param file_id Index of the descriptor to close in g_files.
 *
 * Files should be closed when done with in order to free up file descriptors
 * for other uses.
 */
void adhd_file_close( uint8_t file_id );

/** \} */

/**
 * \addtogroup adhd_tasks ADHD Tasks
 * \brief Simple task manager and scheduler for the VM.
 * \{
 */

/**
 * \addtogroup adhd_tasks_errors ADHD Tasks Errors
 * \brief 
 * \{
 */

/*! \brief No file structures are available in g_files to hold a new file. */
#define ADHD_ERROR_NO_TASKS_AVAIL -1
/*! \brief The specified task was not found to be running. */
#define ADHD_ERROR_TASK_NOT_FOUND -2

/* \} */

#ifndef ADHD_TASKS_MAX
/**
 * \brief Maximum number of tasks the system can run at once.
 *
 * This can be overridden by the build system, though a default is provided.
 */
#define ADHD_TASKS_MAX 3
#endif /* !ADHD_TASKS_MAX */

/*! \brief The given task can gather input and display output to the console. */
#define ADHD_TASK_FLAG_FOREGROUND   0x01

/**
 * \brief Task descriptor holding info on a running task.
 */
struct ADHD_TASK {
   /*! \brief Bitfield indicating special status this task may have. */
   uint8_t flags;
   /*! \brief VM execution information. */
   struct VM_PROC proc;
   /*! \brief Index of the open file entry for this task's executable in
    *         g_files.
    */
   uint8_t file_id;
};

/**
 * \brief Initialize the task and file managers. Should be called on boot.
 */
void adhd_init();

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

/**
 * \brief Execute the next instruction in a running task.
 * \param pid The index of the task in g_tasks to execute.
 *
 * Each run of this function executes a "tick" for a given task. The kernel
 * should call this once for each task in a loop to continue execution of
 * user tasks.
 */
void adhd_task_execute_next( TASK_PID pid );

/**
 * \brief Kill the task with the given task ID.
 * \param pid The index of the task to kill in g_tasks.
 *
 * This frees memory assigned to the task and closes its executable file.
 * Other files may still be left open.
 */
void adhd_task_kill( TASK_PID pid );

#ifdef ADHD_C
struct ADHD_TASK g_tasks[ADHD_TASKS_MAX];
#else
/*! \brief Global tasks descriptor table. */
extern struct ADHD_TASK g_tasks[ADHD_TASKS_MAX];
#endif

/** \} */

#endif /* ADHD_H */

