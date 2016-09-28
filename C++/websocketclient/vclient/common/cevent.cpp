#include "cevent.h"
#include "../common/log.h"

CEvent::CEvent()
{
#ifdef _WIN32
    m_hd = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(NULL == m_hd)
    {
        LOG_ERR("create event failed. reason:%d", GetLastError());
    }
#else
    m_obj = (struct wait_obj *) malloc(sizeof(struct wait_obj));
    m_obj->pipe_fd[0] = -1;
    m_obj->pipe_fd[1] = -1;
    if (pipe(m_obj->pipe_fd) < 0)
        LOG_ERR("%s", "init_wait_obj: pipe failed");

    int flags;
    flags = fcntl(m_obj->pipe_fd[0], F_GETFL, 0 );
    fcntl( m_obj->pipe_fd[0], F_SETFL, flags | O_NONBLOCK );

#endif
}

CEvent::~CEvent()
{
#ifdef _WIN32
    if(NULL == m_hd)
    {
        LOG_ERR("%s", "NULL == m_hd");
    }
    CloseHandle(m_hd);
#else
    if (m_obj)
    {
        if (m_obj->pipe_fd[0] != -1)
        {
            close(m_obj->pipe_fd[0]);
            m_obj->pipe_fd[0] = -1;
        }
        if (m_obj->pipe_fd[1] != -1)
        {
            close(m_obj->pipe_fd[1]);
            m_obj->pipe_fd[1] = -1;
        }
        free(m_obj);
    }
#endif
}

int CEvent::wait(int iTimeout/* = -1*/)
{
#ifdef _WIN32
    if(NULL == m_hd)
    {
        LOG_ERR("%s", "NULL == m_hd");
        return -1;
    }
    DWORD iRet;
    if(-1 == iTimeout)
        iRet = WaitForSingleObject(m_hd, INFINITE);
    else
        iRet = WaitForSingleObject(m_hd, iTimeout);
    if (WAIT_TIMEOUT == iRet)
    {
        LOG_INFO("%s", "WaitForSingleObject WAIT_TIMEOUT");
    }
    else if(WAIT_OBJECT_0 == iRet)
    {
        LOG_INFO("%s", "WaitForSingleObject WAIT_OBJECT_0(The state of the specified object is signaled)");
    }
    else if(WAIT_FAILED == iRet)
    {
        LOG_ERR("WaitForSingleObject WAIT_FAILED. reason:%d", GetLastError());
        return -5;
    }
    else
    {
        LOG_INFO("WaitForSingleObject return value:%d", iRet);
    }
    return 0;
#else  
    int max;
    int rv;
    struct timeval time;
    struct timeval * ptime;
    fd_set fds;
    ptime = 0;
    if (iTimeout >= 0)
    {
        time.tv_sec = iTimeout / 1000;
        time.tv_usec = (iTimeout * 1000) % 1000000;
        ptime = &time;
    }
    max = 0;
    FD_ZERO(&fds);

    if(m_obj)
    {
        FD_SET(m_obj->pipe_fd[0], &fds);
        if(m_obj->pipe_fd[0] > max)
            max = m_obj->pipe_fd[0];
    }
    rv = select(max + 1, &fds, 0, 0, ptime);
    return rv;
#endif

}

int CEvent::clear()
{
#ifdef _WIN32
    if(NULL == m_hd)
    {
        LOG_ERR("%s", "NULL == m_hd");
        return -1;
    }
    BOOL iRet = SetEvent(m_hd);
    if(FALSE == iRet)
    {
        LOG_ERR("SetEvent failed. reason:%d", GetLastError());
        return -5;
    }
    return 0;
#else
    int len;
    len = write(m_obj->pipe_fd[1], "sig", 4);
    if (len != 4)
    {
        LOG_ERR("%s", "set_wait_obj: error");
        return -5;
    }
    return 0;
#endif
}

int CEvent::setSignaled()
{
#ifdef _WIN32
    return clear();
#else
    return clear();
#endif

}

int CEvent::setUnSignaled()
{
#ifdef _WIN32
    if(NULL == m_hd)
    {
        LOG_ERR("%s", "NULL == m_hd");
        return -1;
    }
    BOOL iRet = ResetEvent(m_hd);
    if(FALSE == iRet)
    {
        LOG_ERR("ResetEvent failed. reason:%d", GetLastError());
        return -5;
    }
    return 0;
#else
    int len;
    len = read(m_obj->pipe_fd[0], &len, 4);
    if (len != 4)
    {
        LOG_ERR("%s", "chan_man_clear_ev: error");
        return -5;
    }
    return 0;
#endif
}
