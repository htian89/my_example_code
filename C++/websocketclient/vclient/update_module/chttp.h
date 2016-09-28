#ifndef CHTTP_H
#define CHTTP_H
#include "ctcpsocket.h"

const int BUFSIZE = 10000;

namespace TCPSOCKET
{
enum httpRequestType{
    GET,
    POST
};
}

class Chttp
{
public:
    Chttp(std::string ip, int port);

    void checkVersion(std::string &version);

protected:
    void httpRequestHeader(const std::string &uri, const std::string &input, TCPSOCKET::httpRequestType httpType, std::string &httpHeader);
    void parseVersion(const char *recvbuf, std::string &version);

private:
    std::string m_ip;
    int m_port;
};

#endif // CHTTP_H
