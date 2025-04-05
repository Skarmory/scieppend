#include "scieppend/core/tasking.h"

#include "scieppend/core/concurrent/futex.h"
#include "scieppend/core/list.h"
#include "scieppend/core/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <threads.h>
#include <time.h>

// CONSTS

#define C_MAX_THREADS 8

// FDECL

struct _Thread;
static void _thread_init(struct _Thread* thread, struct Tasker* tasker);
static void _thread_start_task(struct _Thread* thread);
static void _thread_execute_task(struct _Thread* thread);
static void _thread_end_task(struct _Thread* thread);
static void _thread_free_task(struct _Thread* thread);
static int  _thread_update(void* thread);

// STRUCTS

enum _ThreadState
{
    THREAD_STATE_EXECUTING,
    THREAD_STATE_IDLE,
    THREAD_STATE_STOPPING,
    THREAD_STATE_STOPPED
};

struct _Thread
{
    struct Tasker* tasker;
    thrd_t         thread;
    atomic_int     state;
    struct Task*   task;
    int            id;
};

struct Tasker
{
    atomic_int     task_count;
    atomic_bool    kill;

    struct _Thread worker_threads[C_MAX_THREADS];
    mtx_t          tasker_lock;
    cnd_t          work_signal;
    cnd_t          task_complete_signal;

    struct List    pending_list;
};

struct Task
{
    task_func          func;
    task_callback_func cb_func;
    void*              args;
    atomic_int         status;
    char               name[256];
};

// INTERNAL FUNCS

/**
 * Set initial state for a worker thread.
 */
static void _thread_init(struct _Thread* thread, struct Tasker* tasker)
{
    thread->tasker = tasker;
    thread->state = THREAD_STATE_IDLE;

    thrd_create(&thread->thread, _thread_update, thread);
}

/**
 * SHOULD ONLY BE CALLED BY A WORKER THREAD
 * Thread should have locked the pending list mutex at this point.
 * Pop task from list and set thread state to executing.
 */
static inline void _thread_start_task(struct _Thread* thread)
{
    thread->task = list_pop_head(&thread->tasker->pending_list);
    thread->state = THREAD_STATE_EXECUTING;
}

/**
 * SHOULD ONLY BE CALLED BY A WORKER THREAD
 * Loops executing the given task until task returns a non-executing state.
 */
static inline void _thread_execute_task(struct _Thread* thread)
{
    thread->task->status = TASK_STATUS_EXECUTING;
    while(thread->task->status == TASK_STATUS_EXECUTING)
    {
        thread->task->status = thread->task->func(thread->task->args);
    }
}

/**
 * SHOULD ONLY BE CALLED BY A WORKED THREAD
 * Free the task and set thread task to NULL
 */
static void _thread_free_task(struct _Thread* thread)
{
    task_free(thread->task);
    thread->task = NULL;
}

/**
 * SHOULD ONLY BE CALLED BY A WORKER THREAD
 * Call the task's callback, if it has one.
 * Set the thread state to idle and signal the tasker that a task has been completed.
 * Wakes the futex that might be waited upon.
 */
static void _thread_end_task(struct _Thread* thread)
{
    //log_format_msg(LOG_DEBUG, "Worker thread %d ending task: %s", thread->id, thread->task->name);

    if(thread->task->cb_func)
    {
        thread->task->cb_func(thread->task->args);
    }

    --thread->tasker->task_count;
    futex_wake(&thread->task->status, 1);
    thread->task = NULL;
    thread->state = THREAD_STATE_IDLE;

    cnd_signal(&thread->tasker->task_complete_signal);
}

/**
 * Main loop for a worker thread.
 * Lock and check for tasks. If no tasks, wait on signal from the tasker that new tasks are added.
 * Atomic compare on state swaps in case the tasker stops the thread.
 */
static int _thread_update(void* t)
{
    struct _Thread* thread = t;
    while(!atomic_load_explicit(&thread->tasker->kill, memory_order_acquire))
    {
        mtx_lock(&thread->tasker->tasker_lock);

        while(thread->tasker->task_count == 0)
        {
            // No pending tasks, wait for work signal
            cnd_wait(&thread->tasker->work_signal, &thread->tasker->tasker_lock);

            if(atomic_load_explicit(&thread->tasker->kill, memory_order_acquire))
            {
                mtx_unlock(&thread->tasker->tasker_lock);
                goto thread_update_exit_label;
            }
        }

        _thread_start_task(thread);

        mtx_unlock(&thread->tasker->tasker_lock);

        // Check still idle, if so set executing, else stopped so break
        if(thread->tasker->kill)
        {
            _thread_free_task(thread);
            goto thread_update_exit_label;
        }

        _thread_execute_task(thread);
        _thread_end_task(thread);
    }

thread_update_exit_label:
    thread->state = THREAD_STATE_STOPPED;
    thrd_exit(0);
}

// EXTERNAL FUNCS

struct Tasker* tasker_new(void)
{
    struct Tasker* tasker = malloc(sizeof(struct Tasker));
    tasker->task_count = 0;
    tasker->kill = false;
    list_init(&tasker->pending_list);

    mtx_init(&tasker->tasker_lock, mtx_plain);
    cnd_init(&tasker->work_signal);
    cnd_init(&tasker->task_complete_signal);

    for(int i = 0; i < C_MAX_THREADS; ++i)
    {
        _thread_init(&tasker->worker_threads[i], tasker);
        tasker->worker_threads[i].id = i;
    }

    return tasker;
}

void tasker_free(struct Tasker* tasker)
{
    atomic_store_explicit(&tasker->kill, true, memory_order_release);

    mtx_lock(&tasker->tasker_lock);
    cnd_broadcast(&tasker->work_signal);
    mtx_unlock(&tasker->tasker_lock);

    for(int tidx = 0; tidx < C_MAX_THREADS; ++tidx)
    {
        thrd_join((&tasker->worker_threads[tidx])->thread, NULL);
    }

    mtx_destroy(&tasker->tasker_lock);
    cnd_destroy(&tasker->work_signal);

    list_free_data(&tasker->pending_list, &task_free_wrapper);

    free(tasker);
}

bool tasker_add_task(struct Tasker* tasker, struct Task* task)
{
    if(task->status != TASK_STATUS_NOT_STARTED)
    {
        return false;
    }

    // Add task to pending list
    mtx_lock(&tasker->tasker_lock);
    {
        list_add(&tasker->pending_list, task);
        ++tasker->task_count;
    }
    mtx_unlock(&tasker->tasker_lock);

    cnd_signal(&tasker->work_signal);

    return true;
}

void tasker_sync(struct Tasker* tasker)
{
    mtx_lock(&tasker->tasker_lock);
    while(tasker->task_count > 0)
    {
        cnd_wait(&tasker->task_complete_signal, &tasker->tasker_lock);
    }
    mtx_unlock(&tasker->tasker_lock);
}

void tasker_log_state(struct Tasker* tasker)
{
    log_msg(LOG_DEBUG, "Tasker state:");
    log_push_indent(LOG_ID_DEBUG);
    log_format_msg(LOG_DEBUG, "Task Count: %d", tasker->task_count);

    log_msg(LOG_DEBUG, "Threads:");
    log_push_indent(LOG_ID_DEBUG);
    for(int i = 0; i < C_MAX_THREADS; ++i)
    {
        log_format_msg(LOG_DEBUG, "Thread %d", tasker->worker_threads[i].id);
        log_push_indent(LOG_ID_DEBUG);
        log_format_msg(LOG_DEBUG, "State: %d", tasker->worker_threads[i].state);
        log_pop_indent(LOG_ID_DEBUG);
    }
    log_pop_indent(LOG_ID_DEBUG);
    log_pop_indent(LOG_ID_DEBUG);
}

struct Task* task_new(char* task_name, task_func func, task_func cb_func, void* args, int size_bytes)
{
    struct Task* task = malloc(sizeof(struct Task));
    task->status = TASK_STATUS_NOT_STARTED;
    task->func = func;
    task->cb_func = cb_func;
    task->args = malloc(size_bytes);
    memcpy(task->args, args, size_bytes);
    snprintf(task->name, sizeof(task->name), "%s", task_name);

    return task;
}

void task_free(struct Task* task)
{
    task_await(task);
    free(task->args);
    free(task);
}

void task_free_wrapper(void* task)
{
    task_free((struct Task*)task);
}

task_func task_get_func(struct Task* task)
{
    return task->func;
}

bool task_is_finished(struct Task* task)
{
    return task->status == TASK_STATUS_SUCCESS || task->status == TASK_STATUS_FAILED;
}

void task_await(struct Task* task)
{
    while(true)
    {
        int status = atomic_load_explicit(&task->status, memory_order_acquire);
        if (status == TASK_STATUS_SUCCESS || status == TASK_STATUS_FAILED)
        {
            break;
        }

        futex_wait(&task->status, TASK_STATUS_EXECUTING);
    }
}

// EXTERNAL VARS

struct Tasker* g_tasker = NULL;
