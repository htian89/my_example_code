#ifndef IPCCLIENT_H
#define IPCCLIENT_H
#ifdef WIN32
#include <winsock2.h>
#include "../common/cthread.h"
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif
#include "cJSON.h"

#include <string>

class IpcClient
{
public:
    IpcClient();
    ~IpcClient();

    int ipcSendMsg(char *msg, int dataLen);
    int ipcRecvMsg(char *msg);
    void ipcClose();
    inline int getSockfd(){return m_sockfd;}
    inline void setConnected(bool isConnected){m_isConnected = isConnected;}
    inline sockaddr_in getSockAddr(){return m_addr;}
    void sendWebsocketNetworkInfo(const char *ip, const char *port);
    void sendWebsocketSessionStatus(const bool status, const char *logonticket);
    void sendWebsocketSeatNumber(const std::string &seat_number);

    static void *ipcClientProcessMsg(void *);

protected:
    void _initialize();

private:
    sockaddr_in m_addr;
#ifdef WIN32
    SOCKET  m_sockfd;
    THREAD_HANDLE m_threadHandle;
#else
    int m_sockfd;
#endif
    bool m_isConnected;
};

#endif // IPCCLIENT_H
