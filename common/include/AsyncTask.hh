/*!============================================================================
 * @file AsyncTask.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 *
 *  Copyright (C) 2018 Beijing Joobot Technologies Inc.
 */

#ifndef  _ASYNC_TASK_HH
#define  _ASYNC_TASK_HH

#include <pthread.h>
#include <semaphore.h>

/** 
 * @synopsis AsyncTask 
 */
class AsyncTask
{
public:
    /* 
     * This method will be called before execute the task for copying data.
     * It gives you a chance to allocate a space for saving the task data.
     * The default action just returns true.
     */
    virtual bool init_task (pthread_t thread_id, int task_id, DWORD add_data) {return true;};


    /* pure virtual method, subclass must implement this method.
     * NOTE: This method will be called in the async thread.
     */
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data) = 0;

    /* 
     * This method will be called after sending message MSG_ASYNC_TASK_FINISHED
     * in the async thread. It gives you a chance to free the result data.
     * the default action is doing nothing.
     */
    virtual void cleanup_task (pthread_t thread_id, void* res) {};

    pthread_t createTask (HWND hwnd, int task_id, DWORD add_data);

protected:
    AsyncTask() {};
    virtual ~AsyncTask() {};

private:
};

#endif   /* #ifndef _ASYNC_TASK_HH */

