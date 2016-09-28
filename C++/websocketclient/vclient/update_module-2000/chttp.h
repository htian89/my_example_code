#ifndef CHTTP_H
#define CHTTP_H
#include <QObject>
#include "ctcpsocket.h"

const int BUFSIZE = 10000;
const int STDBUFSIZE = 1024;
const int BLOCKSIZE = 1024*64;

namespace TCPSOCKET
{
enum httpRequestType{
    GET,
    POST
};
}
class Chttp :public QObject
{
    Q_OBJECT
public:
    Chttp(std::string ip, int port, QObject *parent = NULL);
    void checkVersion(std::string &version);
    int getClientInfo(char filepath[512], int &fileSize);
    int download(char filepath[512], char filename[512], int &filesize);
protected:
    void httpRequestHeader(const std::string &uri, const std::string &input, TCPSOCKET::httpRequestType httpType, std::string &httpHeader);
    void parseVersion(const char *recvbuf, std::string &version);
signals:
    void on_signal_http_download_progress(const char *,double);
private:
    std::string m_ip;
    int m_port;
};

#endif // CHTTP_H
