#include <stdio.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>

#include <signal.h>
#include <execinfo.h>


#include "TestBinderService.h"

using namespace android;


static void signal_handler(int sig)
{
    int status = 0;
    bool do_exit = false;
    printf("signal %d received\n", sig);
    switch(sig) {
        case SIGTERM:
        case SIGINT:
        case SIGKILL:
            status = 1;
            do_exit = true;
        case SIGABRT:
        case SIGBUS:
        case SIGFPE:
        case SIGILL:
        case SIGSEGV:{
            void *array[10];
            int i = 0, size = 0;
            char **messages = (char **)NULL;

            size = backtrace(array, 10);
            printf("stack trace:\n");
            messages = backtrace_symbols(array, size);
            for (i=0; i<size; i++)
            	printf("%s\n", messages[i]);

            status = 1;
            do_exit = true;
            break;
        }
        case SIGHUP:
        default:
        	printf("signal not supported.\n");
            break;
    }

    if (do_exit != 0){
        exit(status);
    }
}

void test_asm(int32_t increment, volatile int32_t *ptr)
{
    __asm__ __volatile__ ("lock; xaddl %0, %1"
                          : "+r" (increment), "+m" (*ptr)
                          : : "memory");
}

int main (int argc, char **argv)
{
	printf("start to test binder!!!\n");

    signal(SIGHUP, signal_handler); /* catch hangup signal */
    signal(SIGTERM, signal_handler); /* catch kill signal */
    signal(SIGKILL, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGBUS, signal_handler);
    signal(SIGFPE, signal_handler);
    signal(SIGILL, signal_handler);
    signal(SIGSEGV, signal_handler);

#if 0
    int32_t test = 0;
	test_asm(2, &test);

	sp<ProcessState> proc(ProcessState::self());
	sp<IServiceManager> sm = defaultServiceManager();
#endif

	sp<ProcessState> proc(ProcessState::self());
	sp<IServiceManager> sm = defaultServiceManager();
	printf("TestBinderService before\n");
	TestBinderService::instantiate();
	printf("TestBinderService End\n");
	ProcessState::self()->startThreadPool();
	IPCThreadState::self()->joinThreadPool();
	return 0;
}
