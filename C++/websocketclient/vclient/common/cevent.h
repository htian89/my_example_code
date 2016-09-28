#ifndef CEVENT_H
#define CEVENT_H

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#endif

struct wait_obj
{
    int pipe_fd[2];
};

class CEvent
{
public:
    CEvent();
    ~CEvent();
    int wait(int iTimeout = -1);//iTimeout==-1 -->infinite
    int clear();
    int setSignaled();
    int setUnSignaled();

private:
#ifdef _WIN32
    HANDLE m_hd;
#else
    wait_obj * m_obj;
#endif

};

#endif // CEVENT_H
