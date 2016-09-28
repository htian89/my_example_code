#include "cmutexop.h"


#ifdef _WIN32
#define VCLIENT_MUTEX HANDLE
#define VCLIENT_MUTEX_INIT(m) m = CreateMutex(NULL, FALSE, NULL)
#define VCLIENT_MUTEX_LOCK(m) WaitForSingleObject(m, INFINITE)
#define VCLIENT_MUTEX_UNLOCK(m) ReleaseMutex(m)
#define VCLIENT_MUTEX_DESTROY(m) CloseHandle(m)
#else
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/wait.h>
#define VCLIENT_MUTEX pthread_mutex_t
#define VCLIENT_MUTEX_INIT(m) pthread_mutex_init(&m, 0)
#define VCLIENT_MUTEX_LOCK(m) pthread_mutex_lock(&m)
#define VCLIENT_MUTEX_UNLOCK(m) pthread_mutex_unlock(&m)
#define VCLIENT_MUTEX_DESTROY(m) pthread_mutex_destroy(&m)
#endif
#include "log.h"


CMutexOp::CMutexOp()
{
    m_pUseCount = new CUseCount();
    if( NULL == m_pUseCount )
    {
        LOG_ERR("%s", "new failed! (m_pUseCount = new CUseCount())");
    }
    VCLIENT_MUTEX_INIT(m_pMutex);
}

CMutexOp::~CMutexOp()
{
    LOG_INFO("%s", "~CMutexOp has called");
    if( NULL == m_pUseCount )
    {
        LOG_ERR("%s", "exceptions:NULL == m_pUseCount");
        return;
    }
    LOG_INFO("count:%d", m_pUseCount->m_UseCount);
    release();
}

CMutexOp::CMutexOp(const CMutexOp& mutex_in)
{
    LOG_INFO("%s", "copy constructor has called");
    (mutex_in.m_pUseCount->m_UseCount)++;
    this->m_pMutex = mutex_in.m_pMutex;
    this->m_pUseCount = mutex_in.m_pUseCount;
}

CMutexOp CMutexOp::operator =(const CMutexOp& mutex_in)
{
    LOG_INFO("%s", "operator = has called");
    (mutex_in.m_pUseCount->m_UseCount)++;
    if(NULL != this->m_pUseCount)
    {
        release();
    }
    this->m_pMutex = mutex_in.m_pMutex;
    this->m_pUseCount = mutex_in.m_pUseCount;
    return *this;
}

int CMutexOp::release()
{
    if(NULL != this->m_pUseCount)
    {
        if( --((*m_pUseCount).m_UseCount) == 0)
        {
            if( NULL != m_pUseCount)
            {
                delete m_pUseCount;
                m_pUseCount = NULL;
#ifdef WIN32
                if(NULL != m_pMutex)
                {
                    VCLIENT_MUTEX_DESTROY(m_pMutex);
                    LOG_INFO("%s", "has released data");
                }
                m_pMutex = NULL;
#else
                VCLIENT_MUTEX_DESTROY(m_pMutex);
                LOG_INFO("%s", "has released data");
#endif

            }
            else
            {
                return 0;
            }
        }
    }
    return 0;
}

int CMutexOp::lock()
{
    VCLIENT_MUTEX_LOCK(m_pMutex);
    return 0;
}

int CMutexOp::unlock()
{
//    LOG_DEBUG("%s","before unlock");
    VCLIENT_MUTEX_UNLOCK(m_pMutex);
//    LOG_DEBUG("%s","after unlock");
    return 0;
}
