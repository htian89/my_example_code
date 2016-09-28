#ifndef CTHREADPOOL_H
#define CTHREADPOOL_H
#include <map>
#include <vector>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#define VCLIENT_THREAD HANDLE
#define VCLIENT_THREAD_CREATE(thread, attr, start, arg) \
    thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start, arg, 0, NULL)
#define VCLIENT_THREAD_RELEASE(m) CloseHandle(m)
#else
#include <pthread.h>
#define VCLIENT_MUTEX pthread_mutex_t
#define VCLIENT_THREAD_CREATE(thread, attr, start, arg) pthread_create(&thread, attr, start, arg)
#define THREAD_DETACH(m) pthread_detach(m)
#define THREAD_WAIT(m) pthread_join(m)
#define VCLIENT_THREAD_RELEASE(m) ;
#define VCLIENT_THREAD pthread_t
#define Sleep(_minisecond)  sleep(_minisecond/1000)
#endif

#include "../common/cmutexop.h"
#include "../common/ds_access.h"
//define some structure needed
//typedef int (*pThreadFun)(void*);
//typedef void* FUN_THREAD;
typedef void* (*FUN_THREAD)(void*);

struct ThreadInfo
{
    /*
     *description:
     *  this structure is used to save thread infos, the info is used to control the thread.
     *parameter:
     *  pvoid(void*)[in]:
     *      A pointer to the parameter passed to thread, currently the parameter keeps all
     *      the info that the thread needs. we can change the values in this parameter to
     *      control the thread's action
     *  hd_thread(VCLIENT_THREAD)[in]:
     *      A handle to the thread.(currently not used)
     **/
    int iOptype;
    void* pvoid;
    int iUserRequestStop;  //0: no 1:yes
    VCLIENT_THREAD hd_thread;
};
typedef struct ThreadInfo ST_THREAD_INFO;

typedef std::vector<ST_THREAD_INFO> VECTOR_THREAD_INFO;

class CThreadPool
{
public:
    CThreadPool();
    ~CThreadPool();
    int createThread(int opType, FUN_THREAD, void*, taskUUID* pTaskUuid);
    int threadFinished(taskUUID taskUuid, int* piUserReqestStop);
    int changeThreadInfo(taskUUID taskUuid);
    int cancelThread(){return 0;}//current not realized


private:
    //std::map<int, VECTOR_THREAD_INFO> m_threadmap;
    std::map<int, ST_THREAD_INFO> m_threadmap;
    CMutexOp m_mutex;
    int iThreadPollReleased; //1:~CThreadPool has called  0:
};

#endif // CTHREADPOOL_H
