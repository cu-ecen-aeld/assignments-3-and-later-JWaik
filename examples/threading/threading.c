#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// Optional: use these functions to add debug or error prints to your application
// #define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    const int wait_obtain_ms = thread_func_args->wait_to_obtain_ms;
    const int wait_release_ms = thread_func_args->wait_to_release_ms;

    DEBUG_LOG("(threadfunc) Wait to obtain %i ms", wait_obtain_ms);
    usleep(wait_obtain_ms*1000); // in microsecond

    // DEBUG_LOG("(threadfunc) Check mutex addr %p", (void *)thread_func_args->mutex);
    int res = pthread_mutex_lock(thread_func_args->mutex);
    // int res = 0;
    if (!res) {
        DEBUG_LOG("(threadfunc)Mutex-lock success");
    } else {
        ERROR_LOG("(threadfunc)Mutex-lock failed!, Error(%i)", res);
        return NULL;
    }

    DEBUG_LOG("Wait to release %i ms", wait_release_ms);
    usleep(wait_release_ms*1000);

    res = pthread_mutex_unlock(thread_func_args->mutex);
    if (!res) {
        DEBUG_LOG("(threadfunc)Mutex-unlock success");
        thread_func_args->thread_complete_success = true;
        return (void *)thread_func_args;
    } else {
        ERROR_LOG("(threadfunc)Mutex-unlock failed!, Error(%i)", res);
    }

    return NULL;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    // Requirements
    // 1.should not block for the thread to complete. -> Do not call join
    // 2.should use dynamic memory allocation for thread_data structure passed into the thread. -> malloc
    // 3.number of threads active should be limited only by the amount of available memory. -> it's auto check with pthread_create
    // 4.The thread started should return a pointer to the thread_data structure when it exits, which can be used
    // to free memory as well as to check thread_complete_success for successful exit. -> return pointer in threadfunc
    // 5. If a thread was started succesfully @param thread should be filled with the pthread_create thread ID
    // coresponding to the thread which was started. -> it's auto fill with pthread_create
    // 6. return true if the thread could be started, false if a failure occurred.

    // DEBUG_LOG("(start_thread) Check mutex addr %p", (void *)mutex);

    DEBUG_LOG("(start_thread) malloc size %i byte(s)", (int)sizeof(struct thread_data));
    struct thread_data *thread_data_var = (struct thread_data *)malloc(sizeof(struct thread_data)); // It will be freed in Test_threading.c 
    thread_data_var->mutex = mutex;
    thread_data_var->thread_complete_success = false;
    thread_data_var->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_data_var->wait_to_release_ms = wait_to_release_ms;

    bool ret = false;

    // free
    int res = pthread_create(thread, NULL, threadfunc, (void *)thread_data_var);
    if (res != 0) {
        if (res == ENOMEM) {
            ERROR_LOG("(start_thread) Not enough memory to allocate thread stack.");
        } else {
            ERROR_LOG("(start_thread) Error pthread_create %i", res);
        }
    } else {
        DEBUG_LOG("(start_thread) pthread_create OK, tid=%llu", (unsigned long long)*thread);
        ret = true;
    }

    return ret;
}

