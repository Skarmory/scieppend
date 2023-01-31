#ifndef SCIEPPEND_CORE_TASKING_H
#define SCIEPPEND_CORE_TASKING_H

/* A multithreaded tasker.
 */

#include <stdbool.h>

typedef int(*task_func)(void*);
typedef int(*task_callback_func)(void*);

struct Tasker;
struct Task;

enum TaskStatus
{
    TASK_STATUS_NOT_STARTED,
    TASK_STATUS_EXECUTING,
    TASK_STATUS_SUCCESS,
    TASK_STATUS_FAILED
};

/* Create a new tasker and set the initial state.
 * This creates the worker threads for the tasker.
 */
struct Tasker* tasker_new(void);

/* Destroy the tasker and its internal state.
 * This also stops and joins the tasker's worker threads.
 */
void tasker_free(struct Tasker* tasker);

/* Add a task to the pending task list, and signal a thread to awaken.
 * Will return false if the Task is in an invalid state.
 */
bool tasker_add_task(struct Tasker* tasker, struct Task* task);

/* Block until tasker has finished executing all its pending tasks.
 */
void tasker_sync(struct Tasker* tasker);

/* Return true if tasker has any tasks that it has not started yet.
 */
bool tasker_has_pending_tasks(struct Tasker* tasker);

/* Return true if tasker has any tasks that it is currently executing.
 */
bool tasker_has_executing_tasks(struct Tasker* tasker);

/* Return true if tasker has any tasks that it has completed and awaiting integration.
 */
bool tasker_has_completed_tasks(struct Tasker* tasker);

/* Log tasker state for debugging purposes.
 */
void tasker_log_state(struct Tasker* tasker);

/* Create a new task and set its initial state.
 */
struct Task* task_new(char* task_name, task_func func, task_callback_func cb_func, void* args, int size_bytes);

/* Await a task finish and destroy it.
 */
void task_free(struct Task* task);

/* Helper wrapper for task free with void*
 */
void task_free_wrapper(void* task);

/* Return the task's execution function.
 */
task_func task_get_func(struct Task* task);

/* Return true if the task has finished.
 */
bool task_is_finished(struct Task* task);

/* Blocks until the given task is finished.
 */
void task_await(struct Task* task);

extern struct Tasker* g_tasker;

#endif
