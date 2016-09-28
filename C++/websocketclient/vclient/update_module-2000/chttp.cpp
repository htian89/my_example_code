#include "chttp.h"
#include "log.h"
#include "tinyxml/tinyxml.h"
#include <QDebug>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/tcp.h>


#define DESTPATH "/etc/system-update/"

Chttp::Chttp(std::string ip, int port, QObject *parent) :
    QObject(parent),
    m_ip(ip),
    m_port(port)
{
}

void Chttp::httpRequestHeader(const std::string &uri, const std::string &input, TCPSOCKET::httpRequestType httpType, std::string &httpHeader)
{
    if(httpType == TCPSOCKET::GET)
        httpHeader = "GET";
    else
        httpHeader = "POST";
    httpHeader  = httpHeader + " " + uri;
    if(httpType == TCPSOCKET::GET && input.length() > 0)
        httpHeader = httpHeader + "?" + input;
    httpHeader = httpHeader + " HTTP/1.1\r\n";
    //    httpHeader = httpHeader  + "User-Agent: Mozilla/5.0\r\n";

    if(httpType == TCPSOCKET::GET)
    {
        char port[32] = "\0";
        sprintf(port, "%d", m_port);
        httpHeader = httpHeader + "HOST: " + m_ip +":" + port + "\r\n";
        httpHeader = httpHeader + "Connection:close" + "\r\n\r\n";
    }else{
        httpHeader = httpHeader + "HOST: " + m_ip + "\r\n";
        char length[32] = "\0";
        sprintf(length, "%d", input.length());
        httpHeader = httpHeader + "Content-Length: " + length + "\r\n";
        httpHeader = httpHeader + "Content-Type: application/xml\r\n";
        httpHeader = httpHeader + "\r\n";
        httpHeader = httpHeader + input;
    }
    qDebug() <<"httpHeader: \n" <<  httpHeader.c_str();
}

void Chttp::parseVersion(const char *recvbuf, std::string &version)
{
    if(recvbuf == NULL)
        return;
    int errorCode;
    const char *p;
    const char *xml;
    xml = strstr(recvbuf, "\r\n\r\n");
    if(xml == NULL)
        return;
    xml +=4;

    TiXmlDocument doc;
    doc.Parse(xml);
    TiXmlElement* pElmRoot = doc.RootElement();
    if(NULL == pElmRoot)
    {
        LOG_ERR("%s","NULL == pElmRoot");
    }
    else
    {
        TiXmlElement* pElmErr = pElmRoot->FirstChildElement("ErrorCode");
        if(NULL != pElmErr)
        {
            const char* pErrorCode = pElmErr->GetText();
            if(NULL != pErrorCode)
            {
                sscanf(pErrorCode,"%d", &errorCode);
                TiXmlElement* pElmVersion = pElmRoot->FirstChildElement("ClientVersion");
                if(0 == errorCode && NULL != pElmVersion)
                {
                    const char* pVersion = pElmVersion->GetText();
                    if(NULL != pVersion)
                    {
                        LOG_INFO("version:%s", pVersion);
                        if((p = strstr(pVersion, "v")) != NULL)
                        {
                            p++;
                            version = std::string(p);
                        }
                    }
                }
            }//NULL != pErrorCode
        }
    }
}

void Chttp::checkVersion(std::string &version)
{
    char recvbuf[BUFSIZE] = "\0";
    CTcpSocket tcpSocket(m_ip, m_port);
    if(tcpSocket.tcpConnect() < 0)
        return;
    std::string uri = "/RestService/Client/GetClientVersion";
    std::string input = "<GetClientVersion><ClientType>1</ClientType></GetClientVersion>";
    std::string httpHeader;
    httpRequestHeader(uri, input, TCPSOCKET::POST, httpHeader);
    tcpSocket.sendMsg(httpHeader.c_str(), httpHeader.size());
    tcpSocket.recvMsg(recvbuf, BUFSIZE);
    qDebug("buffer: \n%s", recvbuf);

    if(strlen(recvbuf) > 0)
        parseVersion(recvbuf, version);
}

int Chttp::getClientInfo(char filepath[512], int &fileSize)
{
    char recvbuf[STDBUFSIZE] = "\0";
    char *buf;
    CTcpSocket tcpSocket(m_ip, m_port);
    if(tcpSocket.tcpConnect() < 0)
        return -1;
    std::string uri = "/GetClientInfo";
    std::string input;
    std::string httpHeader;
    httpRequestHeader(uri, input, TCPSOCKET::POST, httpHeader);
    if(tcpSocket.sendMsg(httpHeader.c_str(), httpHeader.length()) <= 0){
        LOG_ERR("%s", "send get client info msg error");
        return -1;
    }
    if(tcpSocket.recvMsg(recvbuf, sizeof(recvbuf)) <= 0){
        LOG_ERR("%s", "recv get client info msg error");
        return -1;
    }
    qDebug("buffer: \n%s", recvbuf);
    LOG_INFO("%s", recvbuf);
    buf = strstr(recvbuf, "\r\n\r\n")+4;
    TiXmlDocument doc;
    int errorCode = -1;
    doc.Parse(buf);
    TiXmlElement* pElmRoot = doc.RootElement();
    if(NULL == pElmRoot)
    {
        LOG_ERR("%s","NULL == pElmRoot");
        return -1;
    }
    else
    {
        do{
            TiXmlDocument doc;
            doc.Parse(buf);
            TiXmlElement* pElmRoot = doc.RootElement();
            if(NULL == pElmRoot)
            {
                LOG_ERR("%s","NULL == pElmRoot");
                break;
            }
            else
            {
                TiXmlElement* pElmErr = pElmRoot->FirstChildElement("ErrorCode");
                if(NULL == pElmErr)
                {
                    break;
                }
                else
                {
                    const char* pErrorCode = pElmErr->GetText();
                    if(NULL == pErrorCode)
                    {
                        break;
                    }
                    else
                    {
                        sscanf(pErrorCode,"%d", &errorCode);
                        if(0 != errorCode)
                        {
                            break;
                        }
                        else
                        {
                            TiXmlElement* pElmClient = pElmRoot->FirstChildElement("Client");
                            while(pElmClient != NULL){
                                    TiXmlElement *pElmType = pElmClient->FirstChildElement("Name");
                                    if(pElmType == NULL)
                                    {
                                        errorCode = -1;
                                        break;
                                    }
                                    else
                                    {
                                        if(pElmType->GetText() == NULL)
                                        {
                                            errorCode = -1;
                                            break;
                                        }
                                        else
                                        {
                                            if(strcmp(pElmType->GetText(), "fronView2000") == 0)
                                            {
                                                TiXmlElement *pElmPath = pElmClient->FirstChildElement("Path");
                                                if( pElmPath == NULL)
                                                {
                                                    errorCode = -1;
                                                    break;
                                                }
                                                else
                                                {
                                                    if( pElmPath->GetText() == NULL)
                                                    {
                                                        errorCode = -1;
                                                        break;
                                                    }
                                                    else
                                                    {
                                                        strcpy(filepath, pElmPath->GetText());
                                                        qDebug() << "filepath: " << filepath;
                                                        LOG_INFO("filepath: %s", filepath);
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                pElmClient = pElmClient->NextSiblingElement("Client");
                            }
                        }
                    }//NULL != pErrorCode
                }
            }
        }while(0);
    }
    return errorCode;
}


int Chttp::download(char filepath[512], char filename[512], int &filesize)
{
    LOG_INFO("tar.bz2 name is: %s", filename);
    if(strlen(filename) <= 0)
        return -1;
    char recvbuf[BLOCKSIZE] ;
    char buf[BLOCKSIZE];
    memset(recvbuf, 0, BLOCKSIZE);
    memset(buf, 0, BLOCKSIZE);
    CTcpSocket tcpSocket(m_ip, m_port);
    if(tcpSocket.tcpConnect() < 0)
        return -1;
    std::string uri = "/";
    uri += filepath;
    std::string input ;
    std::string httpHeader;
    httpRequestHeader(uri, input, TCPSOCKET::GET, httpHeader);

    if(tcpSocket.sendMsg(httpHeader.c_str(), httpHeader.length()) <=0)
    {
        LOG_ERR("%s", "send download msg error");
        return -1;
    }
    /**
     *HTTP/1.1 200 OK
     *Server: Apache-Coyote/1.1
     *Last-Modified: Wed, 23 Jul 2014 03:12:35 GMT
     *Connection: Keep-Alive
     *Content-Type: application/x-iso9660-image
     *Content-Length: 880803840
     *Date: Thu, 24 Jul 2014 06:50:36 GMT
     *Connection: close
     * #######content:######
     */
    //make sure the socket is connectting ;
    int keepalive = 1;
    int keepidle = 3;
    int keepinterval = 1;
    int keepcount = 3;
    int optval;
    int sockfd = tcpSocket.getSockfd();
    socklen_t optlen = sizeof(optval);
    int ret;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive ));
    if ( ret != 0)
    {
        LOG_ERR("set keepalive error ; %d", errno);
    }
    else
    {
        ret = getsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen);
        if (ret == 0)
            LOG_INFO("keepalive: %d", optval);
    }
    ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle , sizeof(keepidle ));
    if( ret != 0)
    {
        LOG_ERR("set keepidle error ; %d", errno);
    }
    else
    {
        ret = getsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, &optval, &optlen);
        if(ret == 0)
            LOG_INFO("keepidle: %d", optval);
    }
    ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval ));
    if( ret != 0)
    {
        LOG_ERR("set keepintvl error ; %d", errno);
    }
    else
    {
        ret = getsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, &optval, &optlen);
        if(ret == 0)
            LOG_INFO("keepintvl: %d", optval);
    }
    ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount ));
    if( ret != 0)
    {
        LOG_ERR("set keepcnt error ; %d", errno);
    }
    else
    {
        ret = getsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, &optval, &optlen);
        if (ret == 0)
            LOG_INFO("keepcnt: %d", optval);
    }
    LOG_INFO("sizeof(recvbuf) = %d", sizeof(recvbuf));
    //int firstLen = tcpSocket.recvMsg(recvbuf, sizeof(recvbuf));
    int firstLen = tcpSocket.recvMsg(recvbuf, BLOCKSIZE);
    LOG_INFO("len = %d", strlen(recvbuf));
    if (firstLen <= 0)
    {
        LOG_ERR("%s", "recv download msg error");
        return -1;
    }

    /* ________________________________________________________________________ */
    LOG_INFO("firstLen = %d", firstLen);
    LOG_INFO("\n****************************recvMsg: **************************\n%s\n", recvbuf);
    LOG_INFO("len = %d", strlen(recvbuf));
    if(strlen(recvbuf) > 0)
    {
        strcpy(buf, recvbuf);
    }
    //HTTP/1.1 200 OK
    if(strlen(buf) > 0 && strstr(buf, "200") != NULL)
    {
        LOG_INFO("%s", "the install package is exist");
        //eg: Content-Length: 880803840
        char fileLength[64];
        memset(fileLength, 0, sizeof(fileLength));
        if(strstr(buf, "Content-Length:") != NULL)
        {
            strncpy(fileLength, strstr(buf, "Content-Length:"), sizeof(fileLength));
            fileLength[strlen(fileLength) - strlen(strstr(fileLength, "\r\n"))] = '\0';
            sscanf(fileLength+strlen("Content-Length: "), "%d", &filesize);
            LOG_INFO("filesize = %d\n", filesize);
        }
    }
    else
    {
        LOG_ERR("%s", "the install package is not  exist");
        return -1;
    }
    memset(buf, 0, BLOCKSIZE);
    memcpy(buf, strstr(recvbuf, "\r\n\r\n") +4,  (int )(BLOCKSIZE - (strstr(recvbuf, "\r\n\r\n") +4 - recvbuf))); //解决下载错误问题
    //strcpy(buf , strstr(recvbuf, "\r\n\r\n") +4);
    //LOG_INFO("buf is \n%s", buf);
    /*may be some env make strlen fail;
     * firstLen also has package;
     * */
    LOG_INFO("(1) firstLen = %d", firstLen);
    firstLen -= strlen(recvbuf) - strlen(buf);
    LOG_INFO("strlen(recvbuf) = %d", strlen(recvbuf));
    LOG_INFO("strlen(buf) = %d", strlen(buf));
    LOG_INFO("firstLen -= strlen(recvbuf) - strlen(buf) = %d", firstLen);
    //local file path eg: /etc/systemupdate/$filename;
    char dstfilepath[512];
    memset(dstfilepath, 0, sizeof(dstfilepath));
    strcpy(dstfilepath, DESTPATH);
    strcat(dstfilepath, filename);
    int fd = open(dstfilepath, O_CREAT | O_TRUNC | O_WRONLY | O_APPEND, 0666);
    if( fd < 0)
        return -1;

    if (write(fd, buf, firstLen) < 0)
    {
        close(fd);
        return -1;
    }
    int lenRecv = 0;
    struct stat status;
    int tmp = 0;
    char monitor[512];
    while(1)
    {
        memset(buf, 0, BLOCKSIZE);
        if((lenRecv = tcpSocket.recvMsg(buf, BLOCKSIZE)) < 0)
        {
            LOG_ERR("%s", "recv download package failed");
            close(fd);
            return -1;
        }
        else if(lenRecv == 0)
            break;
        LOG_INFO("write------------------------------>");
        LOG_INFO("lenRecv = %d", lenRecv);
        LOG_INFO("recvMsg: \n %s", buf);
        //monitor the download progress;
        if(write(fd, buf, lenRecv) < 0)
        {
            close(fd);
            return -1;
        }
        stat(dstfilepath, &status);
        if((status.st_size - tmp) >= 1024*1024*10)
        {
            memset(monitor, 0, sizeof(monitor));
            tmp = status.st_size;
            //10M load note ui add progress;
            emit on_signal_http_download_progress(monitor,(double)tmp/(double)filesize);
        }
    }
    close(fd);
    return 0;
}
