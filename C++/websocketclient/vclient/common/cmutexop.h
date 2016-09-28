#ifndef CMUTEXOP_H
#define CMUTEXOP_H



#ifdef _WIN32
#include <windows.h>
#define VCLIENT_MUTEX HANDLE
#else
#include <pthread.h>
#define VCLIENT_MUTEX pthread_mutex_t
#endif

class CUseCount
{
public:
    CUseCount():m_UseCount(1){}
    ~CUseCount(){}
    int m_UseCount;

};

class CMutexOp
{
    friend class CUseCount;
public:
    CMutexOp();
    CMutexOp(const CMutexOp&);
    CMutexOp operator =(const CMutexOp&);
    ~CMutexOp();
    int lock();
    int unlock();

private:
    int release();

    VCLIENT_MUTEX m_pMutex;
    CUseCount* m_pUseCount;


};



#endif // CMUTEXOP_H
