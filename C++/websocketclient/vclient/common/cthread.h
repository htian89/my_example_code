#ifndef CTHREAD_H
#define CTHREAD_H

//typedef void* FUN_THREAD;
typedef void* (*FUN_THREAD)(void*);
typedef void* THREAD_HANDLE;
#ifdef WIN32
    typedef void* THREAD_ATTR;
#else
#include <pthread.h>
    typedef pthread_attr_t* THREAD_ATTR;
#endif
class CThread
{
public:
    CThread();
    static int createThread(THREAD_HANDLE*, THREAD_ATTR, FUN_THREAD, void*);
    static int threadRelease(const THREAD_HANDLE);
};

#endif // CTHREAD_H
