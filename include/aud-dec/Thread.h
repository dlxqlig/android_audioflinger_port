#ifndef _ATHREAD_H_
#define _ATHREAD_H_

#include <pthread.h>

enum ThreadPriority {
    THREAD_PRIORITY_IDLE = -1,
    THREAD_PRIORITY_NORMAL = 0,
    THREAD_PRIORITY_ABOVE_NORMAL = 1,
    THREAD_PRIORITY_HIGH = 2,
};

class AThread 
{
protected:
    pthread_mutex_t m_lock;
    pthread_t m_thread;
    volatile bool m_running;
    volatile bool m_bStop;

private:
    static void *Run(void *arg);

public:
    AThread();
    ~AThread();

    bool Create(ThreadPriority priority);
    virtual void Process() = 0;
    bool Running();
    pthread_t ThreadHandle();
    bool StopThread();
};

#endif
