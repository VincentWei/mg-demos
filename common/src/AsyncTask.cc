/*! ============================================================================
 * @file AsyncTask.cc 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 *
 *  Copyright (C) 2018 Beijing Joobot Technologies Inc.
 */

#include "global.h"

#include "AsyncTask.hh"

typedef struct _TASK_INFO {
    AsyncTask* task;
    HWND hwnd;
    int task_id;
    DWORD add_data;
    sem_t *wait;
} TASK_INFO;

static void* task_entry (void* data)
{
    TASK_INFO task_info;

    memcpy (&task_info, data, sizeof (TASK_INFO));
    pthread_t self = pthread_self ();

    bool ret = task_info.task->init_task (self, task_info.task_id, task_info.add_data);
    sem_post (task_info.wait);

    if (ret) {
        void* res = task_info.task->execute_task (self, task_info.task_id, task_info.add_data);
        SendMessage (task_info.hwnd, MSG_ASYNC_TASK_FINISHED, (WPARAM)task_info.task_id, (LPARAM)res);
        task_info.task->cleanup_task (self, res);
    }
    else {
        SendMessage (task_info.hwnd, MSG_ASYNC_TASK_FINISHED, (WPARAM)task_info.task_id, 0);
    }

    return NULL;
}

pthread_t AsyncTask::createTask (HWND hwnd, int task_id, DWORD add_data)
{
    pthread_t task_thread;
    sem_t wait;
    TASK_INFO task_info = {this, hwnd, task_id, add_data, &wait};

    sem_init (&wait, 0, 0);
    pthread_create (&task_thread, NULL, task_entry, &task_info);
    pthread_detach (task_thread);
    sem_wait (&wait);
    sem_destroy (&wait);

    return task_thread;
}

