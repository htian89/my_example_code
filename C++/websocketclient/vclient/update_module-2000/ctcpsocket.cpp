#include "ctcpsocket.h"
#include "log.h"

#include <string.h>
#include <mxml.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

CTcpSocket::CTcpSocket(std::string ip, int port) :
    m_ip(ip),
    m_port(port)
{
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_sockfd < 0)
    {
        LOG_ERR("create tcp socket error!");
        return ;
    }

    memset(&m_serverAddr, 0, sizeof(sockaddr_in));
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(m_port);
    m_serverAddr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    if(m_serverAddr.sin_addr.s_addr == INADDR_NONE)
    {
        LOG_ERR("Parse the ip address error!");
        struct hostent *ent;
        ent = gethostbyname(m_ip.c_str());
        if (ent == 0)
            return;

        memcpy(&m_serverAddr.sin_addr.s_addr, ent->h_addr, ent->h_length);
    }
}

CTcpSocket::~CTcpSocket()
{
    closeFd();
}

int CTcpSocket::tcpConnect()
{
    if(m_sockfd <=0)
        return -1;
    while( (m_isConnected = connect(m_sockfd, (sockaddr *)&m_serverAddr, sizeof(sockaddr))) < 0)
    {
        if(errno == EINTR)
            continue;
        else
        {
            LOG_ERR("connect to server error! errorCode = %d", errno);
            return -1;
        }
    }
    return 0;
}

int CTcpSocket::sendMsg(const char *msg, int dataLen)
{
    if(msg == NULL || m_isConnected < 0)
        return 0;
    int sendLen = 0;
    int realDataLen = strlen(msg);
    const char *p = msg;
    while(realDataLen>0)
    {
        sendLen = send(m_sockfd, p, dataLen, 0);
        if(sendLen < 0)
        {
            if(errno == EINTR)
                continue;
            else
            {
                LOG_ERR("send msg error, errorCode=", errno);
                return -1;
            }
        }
        p = p+sendLen;
        realDataLen = realDataLen - sendLen;
    }

    return 1;
}

int CTcpSocket::recvMsg(char *outMsg, int dataLen)
{
    if(outMsg == NULL || m_isConnected<0)
        return 0;
    int recvLen = 0;
    if((recvLen  = recv(m_sockfd, outMsg, dataLen, 0)) < 0)
    {
        LOG_ERR("recv error errorCode = %d", errno);
        return -1;
    }
    return recvLen;
}

void CTcpSocket::closeFd()
{
    if(m_sockfd > 0)
        close(m_sockfd);
}
