#include "ipcclient.h"
#include "../backend/cthreadpool.h"
#include "../ui/desktoplistdialog.h"
#include "../ui/autologindialog.h"
#include <string.h>
#include <QDebug>
#include "common/tcp_message.h"

const int PORT = 6060;
const char *serverIpAddress  = "127.0.0.1";
extern CMessageBox *g_cmessagebox;
char notes[MAX_LEN] = {'\0'};

IpcClient::IpcClient()
{
    _initialize();
}

IpcClient::~IpcClient()
{
    ipcClose();
}

void IpcClient::_initialize()
{
#ifdef WIN32
    m_isConnected = false;
    int iRet;
//    WSADATA wsaData;

//    iRet = WSAStartup(MAKEWORD(1, 1), &wsaData);
//    if(0 != iRet)
//    {
//        LOG_ERR("WSAStartup failed. return value:%d", iRet);
//        return;
//    }
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(INVALID_SOCKET == m_sockfd)
    {
        LOG_ERR("socket failed. reason:%d", GetLastError());
        return;
    }
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = inet_addr(serverIpAddress);
    m_addr.sin_port = htons(PORT);
    if(m_addr.sin_addr.s_addr == INADDR_NONE)
    {
        LOG_ERR("Ipc Server ip is wrong!");
        return;
    }

    iRet = CThread::createThread(&m_threadHandle, NULL, (FUN_THREAD)(&ipcClientProcessMsg), (void*)this);
    if(iRet < 0)
    {
        LOG_ERR("CThread::createThread ipcClientProcessMsg failed. return value:%d", iRet);
    }

#else
    m_isConnected = false;
    memset(&m_addr, 0, sizeof(m_addr));
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_sockfd <= 0)
    {
        LOG_ERR( "create ipc client socket error ");
        return;
    }

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(PORT);
    m_addr.sin_addr.s_addr = inet_addr(serverIpAddress);

    if(m_addr.sin_addr.s_addr == INADDR_NONE)
    {
        LOG_ERR("Ipc Server ip is wrong!");
        return;
    }

    pthread_t thread;
    VCLIENT_THREAD_CREATE(thread, NULL, ipcClientProcessMsg, this);
    THREAD_DETACH(thread);
#endif
}

int IpcClient::ipcSendMsg(char *msg, int dataLen)
{
    if(msg==NULL || !m_isConnected)
        return -1;
    return send(m_sockfd, msg, dataLen, 0);
}

int IpcClient::ipcRecvMsg(char *msg)
{
    if(msg==NULL && !m_isConnected)
        return -1;
    return recv(m_sockfd, msg, 20000, 0);
}

void IpcClient::ipcClose()
{
#ifdef WIN32
    if(INVALID_SOCKET!=m_sockfd)
    {
        int iRet = closesocket(m_sockfd);
        if(iRet != 0)
        {
            LOG_ERR("closesocket failed. reason:%d", WSAGetLastError());
        }
        else
            m_sockfd = INVALID_SOCKET;
    }
   // WSACleanup();
#else
    if(m_sockfd > 0)
         close(m_sockfd);
    m_sockfd=0;
#endif
}

void IpcClient::sendWebsocketSessionStatus(const bool status, const char *logonticket)
{
    MsgHeader header = {CLIENT_TYPE_VCLIENT, VCLIENT_MSGC_SET_SESSION_STATUS};
    vClientMsgcSetSessionStatus data;

    memset(&data, 0, sizeof(data));
    data.status = status;
    uint msg_size = sizeof(vClientMsgcSetSessionStatus) + sizeof(MsgHeader);
    char *msg_out = new char[msg_size];
    if(status)
    {
         memcpy(data.ticket, logonticket, strlen(logonticket));
    }
    memcpy(msg_out, &header, sizeof(header));
    memcpy(msg_out + sizeof(header), &data, sizeof(data));

    if( ipcSendMsg(msg_out, msg_size) > 0)
            ;
    else
        qDebug()  << "send session status error!!";
    delete [] msg_out;
}

void IpcClient::sendWebsocketNetworkInfo(const char *ip, const char *port)
{
    MsgHeader header = {CLIENT_TYPE_VCLIENT, VCLIENT_MSGC_SET_ADDRESS};
    vClientMsgcSetAddress data;

    memset(&data, 0, sizeof(data));
    data.port = atoi(port);
    memcpy(data.ip, ip, strlen(ip));

    uint msg_size = sizeof(vClientMsgcSetAddress) + sizeof(MsgHeader);
    char *msg_out = new char[msg_size];


    memcpy(msg_out, &header, sizeof(header));
    memcpy(msg_out + sizeof(header), &data, sizeof(data));

    if( ipcSendMsg(msg_out, msg_size) > 0)
            ;
    else
        qDebug()  << "send session status error!!";
    delete [] msg_out;
}

void IpcClient::sendWebsocketSeatNumber(const std::string &seat_number)
{
    int32_t header_size = sizeof(MsgHeader);
    int32_t msg_size = header_size + sizeof(vClientMsgcSetSeatNumber);
    char *msg_out = new char[msg_size](0);

    MsgHeader *header = (MsgHeader *)msg_out;
    header->client_type = CLIENT_TYPE_VCLIENT;
    header->msg_type = VCLIENT_MSGC_SET_SEAT_NUMBER;

    vClientMsgcSetSeatNumber *data = (vClientMsgcSetSeatNumber *)(msg_out + header_size);
    memset(data->seat_number, 0, MAX_SIZE);
    strcpy(data->seat_number, seat_number.c_str());

    if( ipcSendMsg(msg_out, msg_size) > 0)
            ;
    else
        qDebug()  << "send session status error!!";
    delete [] msg_out;
}

void *IpcClient::ipcClientProcessMsg(void *arg)
{
    IpcClient *ipcClient = static_cast<IpcClient *>(arg);//(IpcClient *)arg;
    if(ipcClient == NULL)
        return NULL;
/*
 *  try connect to ipc server again and again if failed
 */
    while(1)
    {
#ifdef WIN32
        if(ipcClient->getSockfd() <= 0)
            return NULL;
        sockaddr_in addr_in = ipcClient->getSockAddr();
        int iRet = connect(ipcClient->getSockfd(), (sockaddr *)&addr_in, sizeof(sockaddr));
        if( iRet == SOCKET_ERROR)
        {
            LOG_ERR("connet ipc server address error!!!!!!!!try again reason:%d",  WSAGetLastError());
            Sleep(3000);
        }
        else
            break;
#else
        if(ipcClient->getSockfd() <= 0)
            return NULL;
        sockaddr_in addr_in = ipcClient->getSockAddr();
        if(connect(ipcClient->getSockfd(), (sockaddr *)&addr_in, sizeof(sockaddr)) < 0)
        {
            if(errno==EINTR)
            {
                LOG_ERR("System interupt when connect to ipc server");
                return NULL;
            }
            sleep(3);
        }
        else
            break;
#endif
    }
//    LOG_ERR("connet ipc server address error!!!!!!!");

    ipcClient->setConnected(true);

    LOG_INFO("wait for receving message from ipc server");
    char msg[20000];
    while(1)
    {
        int len = ipcClient->ipcRecvMsg(msg);
        LOG_INFO("receive message :\n%s",  msg);
        if(len <= 0)
        {
            LOG_ERR("receive message len < 0, break");
            break;
        }

        MsgHeader *header = (MsgHeader *)msg;
        LOG_DEBUG("msg type = %d", header->msg_type);

        switch(header->msg_type)
        {
        case VCLIENT_MSG_LOGOFF:
        {
            DesktopListDialog::ipcClientProcessMsg(NULL);
            AutoLoginDialog::ipcClientProcessMsg(NULL);
            break;
        }
        case VCLIENT_MSG_SET_SEAT_NUMBER:
        {
            g_cmessagebox->setSeatNumber();
            break;
        }
        case VCLIENT_MSG_SHOW_SEAT_NUMBER:
        {
            vClientMsgShowSeatNumber *msg_data = (vClientMsgShowSeatNumber *)(msg + sizeof(MsgHeader));
            g_cmessagebox->showSeat = QString(QLatin1String(msg_data->seat_number));
            g_cmessagebox->showSeatNumber();
            break;
        }
        case VCLIENT_MSG_START_BROADCAST:
        {
            break;
        }
        case VCLIENT_MSG_END_BROADCAST:
        {
            break;
        }
        case VCLIENT_MSG_NOTES:
        {
            vClientMsgNotes *msg_data = (vClientMsgNotes *)(msg + sizeof(MsgHeader));
            memset(notes, 0, MAX_LEN);
            memcpy(notes, msg_data->send_notes, strlen(msg_data->send_notes));
            g_cmessagebox->showNotes();
            break;
        }
        default:
            LOG_ERR("unsupport msg type");
            break;
        }
    }

    ipcClient->setConnected(false);

    return NULL;
}
