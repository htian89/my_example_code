#include "cthreadpool.h"
#include "../common/log.h"

using namespace std;

int set_pSessionToNULL(void*pvoid, int itype);////defined in csession.cpp

CThreadPool::CThreadPool()
{
    iThreadPollReleased = 0;
}
CThreadPool::~CThreadPool()
{
    iThreadPollReleased = 1;
    m_mutex.lock();
    for(map<int, ST_THREAD_INFO>::iterator iter = m_threadmap.begin(); iter != m_threadmap.end(); ++iter)
    {
        (iter->second).iUserRequestStop = 1;
        //set_pSessionToNULL((iter->second).pvoid, (iter->second).iOptype);//defined in csession.h
    }
    m_mutex.unlock();
//wait for all thread to finished.
    bool bAllThreadHasFinished = false;
    while(!bAllThreadHasFinished)
    {
        m_mutex.lock();
        if(m_threadmap.size()>0)
            Sleep(200);
        else
            bAllThreadHasFinished = true;
        m_mutex.unlock();
    }
//    LOG_INFO("%s", "+++++++++all thread has finished");
}

int CThreadPool::createThread(int opType, FUN_THREAD pFun, void * pvoid, taskUUID* pTaskUuid)
{
    int iRet = 0;
    if(NULL != pTaskUuid)
    {
        *pTaskUuid = TASK_UUID_NULL;
    }
    VCLIENT_THREAD hd;    
    ST_THREAD_INFO threadInfo;
    threadInfo.iOptype = opType;
    threadInfo.pvoid = pvoid;    
    threadInfo.iUserRequestStop = 0;

    m_mutex.lock();
    if(1 == iThreadPollReleased)//because it it run parallelize. it may has the condition after the destructor has
    {//called but this function also called. this may lead app crashes
        LOG_ERR("%s","thread pool has released. task not added to threadpool");
        set_pSessionToNULL(pvoid, opType);
        m_mutex.unlock();
        return -1;
    }
    VCLIENT_THREAD_CREATE(hd, 0, pFun, pvoid);
#ifdef _WIN32
    if(NULL == hd)
    {
        LOG_ERR("create thread failed:reason:%d",GetLastError());
    }
#endif
    if(NULL != pTaskUuid)
    {
        *pTaskUuid = (taskUUID)hd;
    }
    threadInfo.hd_thread = hd;
    map<int, ST_THREAD_INFO>::iterator iter = m_threadmap.find((int)hd);
    if(m_threadmap.end() != iter)
    {
        m_threadmap.erase(iter);
        LOG_ERR("error may be occured task uuid:%d ", hd);
    }
    m_threadmap.insert(pair<int, ST_THREAD_INFO>((int)hd, threadInfo));
    LOG_INFO("thread STARTED:%d", (int)hd);
    m_mutex.unlock();

//    m_mutex.lock();
//    map<int, VECTOR_THREAD_INFO>::iterator iter = m_threadmap.find(opType);
//    if(m_threadmap.end() == iter)
//    {
//        VECTOR_THREAD_INFO vectorThreadInfo;
//        m_threadmap.insert(pair<int, VECTOR_THREAD_INFO>(opType, vectorThreadInfo));//(map<int, VECTOR_THREAD_INFO>::value_type(opType, vectorThreadInfo));
//        iter = m_threadmap.find(opType);
//        if(m_threadmap.end() == iter)
//        {
//            LOG_ERR("%s", "insert value to map<int, VECTOR_THREAD_INFO> failed");
//            m_mutex.unlock();
//            return -1;
//        }
//    }
//    (iter->second).push_back(threadInfo);
//    LOG_INFO("thread STARTED:%d", (int)hd);
//    m_mutex.unlock();
    return iRet;
}

int CThreadPool::threadFinished(taskUUID taskUuid, int* piUserReqestStop)
{
    if(NULL != piUserReqestStop)
    {
       *piUserReqestStop = 0;
    }
    m_mutex.lock();
    if(1 == iThreadPollReleased)
    {
        LOG_ERR("thread pool has released. but threadfinished has called taskUuid:%d", taskUuid);
        *piUserReqestStop = 1;
        m_mutex.unlock();
        return -1;
    }
    map<int, ST_THREAD_INFO>::iterator iter = m_threadmap.find(taskUuid);
    if(m_threadmap.end() == iter)
    {
        LOG_ERR("doesnot find info for thread:%d", taskUuid);
        m_mutex.unlock();
        return -1;
    }
//    ST_THREAD_INFO ttt= iter->second;
//    VCLIENT_THREAD_RELEASE(iter->second.hd_thread);
    THREAD_DETACH(iter->second.hd_thread);
    if(NULL != piUserReqestStop)
        *piUserReqestStop = iter->second.iUserRequestStop;
    m_threadmap.erase(iter);
    LOG_INFO("thread FINISHED:%d", (int)taskUuid);
    m_mutex.unlock();
//    bool iHasFound = false;
//    m_mutex.lock();
//    int iSize = m_threadmap.size();
//    iSize = iSize;
//    for(map<int, VECTOR_THREAD_INFO>::iterator iter = m_threadmap.begin(); iter != m_threadmap.end(); ++iter)
//    {
//        VECTOR_THREAD_INFO tt = iter->second;
//        tt = tt;
//        int size = (iter->second).size();
//        size = size;

//        for(VECTOR_THREAD_INFO::iterator iter_vec = (iter->second).begin(); iter_vec != (iter->second).end(); ++iter_vec)
//        {
//            if(iter_vec->hd_thread == (VCLIENT_THREAD)taskUuid)
//            {
//                VCLIENT_THREAD_RELEASE(iter_vec->hd_thread);
//                if(NULL != piUserReqestStop)
//                    *piUserReqestStop = iter_vec->iUserRequestStop;
//                (iter->second).erase(iter_vec);
//                iHasFound = true;
//                LOG_INFO("thread FINISHED:%d", (int)taskUuid);
//                break;
//            }
//        }
//    }
//    m_mutex.unlock();
//    if(!iHasFound)
//    {
//        LOG_ERR("not found thread info for %d", taskUuid);
//    }
    return 0;
}

int CThreadPool::changeThreadInfo(taskUUID taskUuid)
{
//    bool iHasFound = false;
    m_mutex.lock();
    if(1 == iThreadPollReleased)
    {
        LOG_ERR("thread pool has released. but changeThreadInfo has called taskUuid:%d", taskUuid);
        m_mutex.unlock();
        return -1;
    }
    map<int, ST_THREAD_INFO>::iterator iter = m_threadmap.find(taskUuid);
    if(m_threadmap.end() == iter)
    {
        LOG_ERR("doesnot find info for thread:%d", taskUuid);
        m_mutex.unlock();
        return -5;
    }
    (iter->second).iUserRequestStop = 1;
    //set_pSessionToNULL((iter->second).pvoid, (iter->second).iOptype);
    m_mutex.unlock();
//    m_mutex.lock();
//    for(map<int, VECTOR_THREAD_INFO>::iterator iter = m_threadmap.begin(); iter != m_threadmap.end(); iter++)
//    {
//        for(int i = 0; i < (iter->second).size(); i++)
//        {
//            if((iter->second)[i].hd_thread == (VCLIENT_THREAD)taskUuid)
//            {
//                (iter->second)[i].iUserRequestStop = 1;
//                iHasFound = true;
//                break;
//            }
//        }
//    }
//    m_mutex.unlock();
//    if(!iHasFound)
//    {
//        LOG_ERR("not found thread info for %d", taskUuid);
//    }
    return 0;
}
