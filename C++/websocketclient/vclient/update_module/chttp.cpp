#include "chttp.h"
#include "log.h"
#include "tinyxml/tinyxml.h"
#include <QDebug>

Chttp::Chttp(std::string ip, int port) :
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
    if(httpType == TCPSOCKET::GET)
        httpHeader = httpHeader + "?" + input;
    httpHeader = httpHeader + " HTTP/1.0\r\n";
    httpHeader = httpHeader  + "User-Agent: Mozilla/5.0\r\n";

    if(httpType == TCPSOCKET::GET)
    {
        char port[32] = "\0";
        sprintf(port, "%d", m_port);
        httpHeader = httpHeader + "HOST: " + m_ip +":" + port + "\r\n";
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
            }
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
    LOG_INFO("revc version: %s", recvbuf);
    qDebug("buffer: \n%s", recvbuf);

    if(strlen(recvbuf) > 0)
        parseVersion(recvbuf, version);
}
