
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "aud-dec/Logger.h"
#include "aud-dec/Thread.h"

#define CMN_MSG ((std::string("AThread::")+ std::string(__FUNCTION__)).c_str())

AThread::AThread()
{
    EntryLogger entry(__FUNCTION__);
    pthread_mutex_init(&m_lock, NULL);
    m_thread = 0;
    m_bStop = false;
    m_running = false;
}

AThread::~AThread()
{
    EntryLogger entry(__FUNCTION__);
    pthread_mutex_destroy(&m_lock);
}

bool AThread::StopThread()
{
    if(!m_running) {
        Logger::LogOut(LOG_LEVEL_DEBUG, "%s - No thread running", CMN_MSG);
        return false;
    }

    m_bStop = true;
    pthread_join(m_thread, NULL);
    m_running = false;

    m_thread = 0;

    Logger::LogOut(LOG_LEVEL_DEBUG, "%s - Thread stopped", CMN_MSG);
    return true;
}

bool AThread::Create(ThreadPriority priority)
{
    EntryLogger entry(__FUNCTION__);
    if(m_running) {
        Logger::LogOut(LOG_LEVEL_ERROR, "%s - Thread already running", CMN_MSG);
        return false;
    }

    m_bStop = false;
    m_running = true;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (priority != THREAD_PRIORITY_NORMAL) {
        if (priority == THREAD_PRIORITY_IDLE) {
            Logger::LogOut(LOG_LEVEL_ERROR, "%s - PRIORITY IDLE not supported.", CMN_MSG);
        }
        else {
            // set real-time round robin thread policy
            if (pthread_attr_setschedpolicy(&attr, SCHED_RR) != 0) {
                Logger::LogOut(LOG_LEVEL_ERROR, "%s - failed to pthread_attr_setschedpolicy.", CMN_MSG);
            }
            struct sched_param param;
            if (pthread_attr_getschedparam(&attr, &param) != 0) {
                Logger::LogOut(LOG_LEVEL_ERROR, "%s - failed to pthread_attr_getschedparam.", CMN_MSG);
            }
            else {
                if (priority == THREAD_PRIORITY_HIGH) {
                    param.sched_priority = 6;// HIGH
                }
                else {
                param.sched_priority = 4; // ABOVE_NORMAL
                }
                if (pthread_attr_setschedparam(&attr, &param) != 0) {
                    Logger::LogOut(LOG_LEVEL_ERROR, "%s - failed to pthread_attr_setschedparam.", CMN_MSG);
                }
            }
        }
    }

    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) != 0) {
        Logger::LogOut(LOG_LEVEL_ERROR, "%s - failed to pthread_attr_setdetachstate.", CMN_MSG);
    }
    pthread_create(&m_thread, &attr, &AThread::Run, this);
    pthread_attr_destroy(&attr);

    Logger::LogOut(LOG_LEVEL_DEBUG, "%s - Thread with id %d started", CMN_MSG, (int)m_thread);
          return true;
}

bool AThread::Running()
{
    return m_running;
}

pthread_t AThread::ThreadHandle()
{
    return m_thread;
}

void *AThread::Run(void *arg)
{
    EntryLogger entry(__FUNCTION__);
    AThread *thread = static_cast<AThread *>(arg);

    thread->Process();

    Logger::LogOut(LOG_LEVEL_DEBUG, "%s - Exited thread with  id %d", CMN_MSG, (int)thread->ThreadHandle());
    pthread_exit(NULL);
}
