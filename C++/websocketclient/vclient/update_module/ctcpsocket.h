#ifndef CTCPSOCKET_H
#define CTCPSOCKET_H
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <string>

class CTcpSocket
{
public:
    CTcpSocket(std::string ip, int port);
    ~CTcpSocket();
    int tcpConnect();
    int sendMsg(const char *msg, int dataLen);
    int recvMsg(char *outMsg, int dataLen);
    void closeFd();

private:
    sockaddr_in m_serverAddr;
    int m_sockfd;
    std::string m_ip;
    int m_port;
    int m_isConnected;

};

#endif // CTCPSOCKET_H
