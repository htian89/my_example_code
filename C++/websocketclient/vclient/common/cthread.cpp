#include "cthread.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
typedef void*(*start_rtn)(void*);
typedef  start_rtn FUN_THREAD_LINUX;
#endif

#include "../common/log.h"

#include <iostream>
using namespace std;


CThread::CThread()
{
}

//success return 0; else ruturn value < 0
int CThread::createThread(THREAD_HANDLE* pHandle, THREAD_ATTR attr, FUN_THREAD pFun, void* arg)
{
#ifdef _WIN32
    HANDLE hd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pFun, arg, 0, NULL);
    if(NULL != pHandle)
        *pHandle = (THREAD_HANDLE)hd;
    if(NULL == hd)
    {
        LOG_ERR("create create thread failed.  handle:%d, reason:%d", hd, GetLastError());
        return -1;
    }
    return 0;
#else
    if(attr == NULL){
        LOG_INFO("%s", "THREAD_ATTR == NULL");
    }
    pthread_t ntid;
    pthread_attr_t attr_t;
    pthread_attr_init (&attr_t);
    pthread_attr_setdetachstate(&attr_t,PTHREAD_CREATE_DETACHED);



    int iRet = pthread_create(&ntid, &attr_t, (FUN_THREAD_LINUX)pFun, arg);

    pthread_attr_destroy(&attr_t);
    if(pHandle!=NULL)
        *pHandle = (THREAD_HANDLE)ntid;
    if(iRet != 0)
    {
        LOG_ERR("create create thread failed. reason:%d", iRet);
        return -1;
    }
    return 0;
#endif
}

int CThread::threadRelease(const THREAD_HANDLE handle)
{
#ifdef _WIN32
    if(NULL != (HANDLE)handle)
    {
        BOOL  bRet = CloseHandle(handle);
        if(bRet == FALSE)
        {
            LOG_ERR("CloseHandle failed. handle:%d  reason:%d", (HANDLE)handle, GetLastError());
            return -1;
        }
    }
    else
    {
        LOG_ERR("%s","NULL == handle. may have error!!!!!");
    }
    return 0;
#else
    if(handle == NULL){
        LOG_INFO("%s","this is linux env");
    }
    return 0;
#endif
}

//#define VCLIENT_MUTEX pthread_mutex_t
//#define VCLIENT_THREAD_CREATE(thread, attr, start, arg) pthread_create(&thread, attr, start, arg)
//#define VCLIENT_THREAD_RELEASE(m) ;

//#define VCLIENT_THREAD HANDLE
//#define VCLIENT_THREAD_CREATE(thread, attr, start, arg)  thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start, arg, 0, NULL)
//#define VCLIENT_THREAD_RELEASE(m) CloseHandle(m)
