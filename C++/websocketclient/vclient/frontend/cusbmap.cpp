#include "cusbmap.h"
#include "../common/cprocessop.h"
#include "../backend/csession.h"
#include "../common/errorcode.h"
#include "../common/log.h"
#include "../backend/cthreadpool.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <QDebug>
using namespace std;
extern bool g_isUsbOccupy; //defined in desktoplistdialog.cpp
#ifdef _WIN32
CUsbMap::CUsbMap()
{
    m_pSession = CSession::GetInstance();
    m_hdSlotVclient = INVALID_HANDLE_VALUE;
    m_hdSlotFusb = INVALID_HANDLE_VALUE;
    m_pProcOp_usbMap = NULL;
    b_hasMapped = false;
}

CUsbMap::~CUsbMap()
{
    if(NULL != m_pProcOp_usbMap)
    {
        writeToSlot(MAIL_SLOT_NAME_FUSB, "free");
        //writeToSlot(m_hdSlotFusb, "free");
        vaccessCloseChannel();
        delete m_pProcOp_usbMap;
        m_pProcOp_usbMap = NULL;
    }
    if(INVALID_HANDLE_VALUE != m_hdSlotVclient)
    {
        std::string str;
        readFromSlot(m_hdSlotVclient,str);
        CloseHandle(m_hdSlotVclient);
        m_hdSlotVclient = INVALID_HANDLE_VALUE;
    }

    if(b_hasMapped)
        g_isUsbOccupy = false;
}

int CUsbMap::launchUsbMap(const APP_LIST& appInfo, const RESOURCE_PARAMETERS& resParam, const NETWORK& netWork, const LAUNCH_RESOURCE_DATA& resData)
{
    int iRet = 0;
    if(strlen(resParam.usb)<=0)
    {
        LOG_ERR("%s", "strlen(m_resParam.usb)<=0");
        return -1;
    }
    g_isUsbOccupy = true;
    b_hasMapped = true;
//create mailslot
    LOG_INFO("m_resParam.usb:\t", resParam.usb);
    m_hdSlotVclient = CreateMailslotA(MAIL_SLOT_NAME_VCLIENT, 0, MAILSLOT_WAIT_FOREVER, NULL);
    if(INVALID_HANDLE_VALUE == m_hdSlotVclient)
    {
        LOG_ERR("%s", "INVALID_HANDLE_VALUE == hdSlot");
        return -5;
    }
    else
    {
//create usbmap process
        m_pProcOp_usbMap = new CProcessOp(APP_FUSB_TCP_PATH, "");
        if(NULL == m_pProcOp_usbMap)
        {
            LOG_ERR("%s", "NULL==m_pProcOp_usbMap");
            return -10;
        }
        iRet = m_pProcOp_usbMap->create();
    }

    PARAM_CHANNELOP_IN param_openChannel;
//get params
    CALLBACK_PARAM_UI stCall_param;
    stCall_param.pUi = NULL;
    stCall_param.errorCode = 0;
    PARAM_SESSION_IN param;
    param.callback_param = &stCall_param;
    param.isBlock = BLOCKED;    

    switch (appInfo.desktopType)
    {
    case VIRTUALAPP:
    case NORMALDESKTOP:
    {
        param_openChannel.str_ip = appInfo.hostname;
        break;
    }
    case DESKTOPPOOL:
    {
        CHECK_DESK_STATE_DATA st_checkResult;
        m_pSession->checkDesktopPoolState(param, appInfo.uuid, &st_checkResult);
        if(0==stCall_param.errorCode && st_checkResult.strIp.size()>0)
        {
            param_openChannel.str_ip = st_checkResult.strIp;
        }
        break;
    }
    case REMOTEDESKTOP:
    {
        CHECK_DESK_STATE_DATA st_checkResult;
        m_pSession->checkRemoteDesktopState(param, appInfo.uuid, &st_checkResult);
        if(0==stCall_param.errorCode && st_checkResult.strIp.size()>0)
        {
            param_openChannel.str_ip = st_checkResult.strIp;
        }
        break;
    }
    default:
    {
        LOG_ERR("unknown desktop type:%d", appInfo.desktopType);
        stCall_param.errorCode = -1;
        break;
    }
    }
    //m_hdSlotFusb = CreateFile(TEXT(MAIL_SLOT_NAME_FUSB), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    m_desktopIp = param_openChannel.str_ip;
    if(stCall_param.errorCode < 0)
    {
        LOG_ERR("checkDesktopPoolState failed, return value:%d", stCall_param);
        std::string str;
        readFromSlot(m_hdSlotVclient,str);
        //writeToSlot(MAIL_SLOT_NAME_FUSB, "free");
        writeToSlot(m_hdSlotFusb, "free");
        if(INVALID_HANDLE_VALUE != m_hdSlotVclient)
        {
            CloseHandle(m_hdSlotVclient);
            m_hdSlotVclient = INVALID_HANDLE_VALUE;
        }
        return stCall_param.errorCode;
    }

    CHANNEL_OP_DATA stChannelData;
    if(NULL == m_pSession)
        m_pSession = CSession::GetInstance();
    m_pSession->openChannel(param, param_openChannel, &stChannelData);
    if(stCall_param.errorCode < 0)
    {
        LOG_ERR("openChannel failed. return value:%d", stCall_param.errorCode);
        return stCall_param.errorCode;
    }
    std::string strData = std::string("data ") + resParam.usb;
    std::string strHost = "host ";
    if(0 == stChannelData.str_ip.size() || stChannelData.str_ip == "127.0.0.1")
    {
        char chPort [MIN_LEN];
        chPort[0] = '\0';
        itoa(stChannelData.iPort, chPort, 10);
        strHost = strHost + netWork.stPresentServer.serverAddress + " port " + chPort;//strHost = strHost + netWork.presentServer + " port " + chPort;
    }
    else
    {
        strHost = strHost + stChannelData.str_ip + " port 3240";
    }
    std::string str;
    readFromSlot(m_hdSlotVclient,str);
//    writeToSlot(m_hdSlotFusb, strData);
//    writeToSlot(m_hdSlotFusb, strHost);
    writeToSlot(MAIL_SLOT_NAME_FUSB, strData);
    Sleep(500);
    writeToSlot(MAIL_SLOT_NAME_FUSB, strHost);

    return 0;
}

int CUsbMap::vaccessCloseChannel()
{
    int iRet = 0;
    if(m_desktopIp.size() <= 0)
    {
        return -1;
    }

    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI stCall_param;
    stCall_param.errorCode = 0;
    PARAM_SESSION_IN param;
    param.callback_param = &stCall_param;
    param.isBlock = BLOCKED;
//int shutdownRes(PARAM_SESSION_IN& st_param_in, char* pResTicket);
    if(NULL == m_pSession)
    {
        LOG_ERR("%s", "NULL==m_pSession");
        return -5;
    }
    PARAM_CHANNELOP_IN stChannelIn;
    CHANNEL_OP_DATA stChannelData;
    stChannelIn.str_ip = m_desktopIp;
    iRet = m_pSession->closeChannel(param, stChannelIn, &stChannelData);
    if(iRet < 0)
    {
        LOG_ERR("shutdownRes failed, return value:", iRet);
        return iRet;
    }
    if(stCall_param.errorCode < 0)
    {
        LOG_ERR("checkDesktopPoolState failed, return value:%d", stCall_param);
        return stCall_param.errorCode;
    }
    return 0;
}

void *CUsbMap::waitUsbMap(void *)
{    
    return 0;
}

int CUsbMap::quit()
{
    int iRet = 0;
    vaccessCloseChannel();
    if(NULL != m_pProcOp_usbMap)
    {
        iRet = m_pProcOp_usbMap->termate();
        LOG_INFO("terminate process return value:%d", iRet);
    }
    return iRet;
}

int CUsbMap::writeToSlot(const HANDLE hd, const std::string str_data)
{
    if(hd == INVALID_HANDLE_VALUE)
    {
         return -1;//Attention: do not change the return value in this condition.will use it to judge whether the slot has been closed
    }

    DWORD dwWriteBytes;
    BOOL fResult = WriteFile(hd, str_data.c_str(), str_data.size(), &dwWriteBytes, NULL);
    //LOG_INFO("write value:%d, return value:%d", str_data.c_str(), fResult);
    //CloseHandle(hd);
    if(FALSE == fResult)
    {
        LOG_ERR("Read file failed reason:%d", GetLastError());
        return -10;
    }
    else
        LOG_INFO("+++++++++++++++++++++:str_data.c_str():%s, size:%d, write size:%d",str_data.c_str(),str_data.size(), dwWriteBytes);
    return 0;
}

int CUsbMap::writeToSlot(const std::string strSlotName, const std::string str_data)
{
    int iTryTimes = 0;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    while(iTryTimes<=4)
    {
        hFile = CreateFileA(strSlotName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE)
        {
            iTryTimes++;
            Sleep(500);
        }
        else
            break;
    }
    if(hFile == INVALID_HANDLE_VALUE)
    {
         LOG_ERR("failed to create file %s ", strSlotName.c_str());
         return -1;//Attention: do not change the return value in this condition.will use it to judge whether the slot has been closed
    }

    DWORD dwWriteBytes;
    char data[MIN_LEN];
    memset(data, 0, MIN_LEN);
    strcpy(data, str_data.c_str());
    BOOL fResult = WriteFile(hFile, data, str_data.size()+1, &dwWriteBytes, NULL);
    LOG_INFO("write value:%s, return value:%d size:%d, write size:%d",str_data.c_str(), fResult, str_data.size(), dwWriteBytes);
    CloseHandle(hFile);
    if(FALSE == fResult)
    {
        LOG_ERR("Read file failed reason:%d", GetLastError());
        return -10;
    }
    return 0;
}

int CUsbMap::readFromSlot(const HANDLE hd, std::string& str_data_read)
{
    int iRet = 0;
    if(INVALID_HANDLE_VALUE == hd)
        return -1;
    char buffer[1024];
    DWORD dwReadBytes = 0;
    memset(buffer, 0, 1024);
    HANDLE hEvent = CreateEvent(NULL,FALSE,FALSE, TEXT("vclient_mailslot"));
    if(!hEvent)
    {
        LOG_ERR("CreateEvent failed. reson:%d", GetLastError());
        return -5;
    }
    OVERLAPPED ov;
    ov.Offset = 0;
    ov.OffsetHigh = 0;
    ov.hEvent = hEvent;
    int iTryTimes = 0;
    while (iTryTimes < 8)
    {
        DWORD  dwNextSize=0, dwMsgCount=0;
        BOOL fResult = GetMailslotInfo(hd, NULL, &dwNextSize, &dwMsgCount, NULL);
        if(FALSE == fResult || MAILSLOT_NO_MESSAGE == dwNextSize)
        {
            LOG_ERR("GetMailslotInfo failed reason:%d", GetLastError());
            iRet = -10;
            iTryTimes++;
            Sleep(500);
            continue;
        }
        fResult = ReadFile(hd, buffer, 1024, &dwReadBytes, &ov);
        if(FALSE == fResult)
        {
            LOG_ERR("Read file failed reason:%d", GetLastError());
            iRet = -10;
            iTryTimes++;
            Sleep(500);
            continue;
        }
        else
        {
            LOG_INFO("date read:%s", buffer);
            str_data_read = buffer;
            break;
        }
    }
    return 0;
}


#else
int CUsbMap::m_iExitCode = 0;

CUsbMap::CUsbMap() :
m_pProcOp_usbMap(NULL)
{
//    signal(SIGCHLD, waitUsbMap);
    m_pSession = CSession::GetInstance();
    b_hasMapped = false;
}

CUsbMap::~CUsbMap()
{
    if(NULL != m_pProcOp_usbMap)
    {
        quit();
        delete m_pProcOp_usbMap;
        m_pProcOp_usbMap = NULL;
    }

    if(b_hasMapped)
        g_isUsbOccupy = false;
}

int CUsbMap::launchUsbMap(const APP_LIST& appInfo, const RESOURCE_PARAMETERS& resParam, const NETWORK& netWork, const LAUNCH_RESOURCE_DATA& resData)
{
    int iRet;
    if(strlen(resParam.usb)<=0)
    {
        LOG_ERR("%s", "strlen(m_resParam.usb)<=0");
        return -1;
    }

    if(g_isUsbOccupy)
        return 0;

    g_isUsbOccupy = true;
    b_hasMapped = true;

    PARAM_CHANNELOP_IN param_openChannel;
//get params
    CALLBACK_PARAM_UI stCall_param;
    stCall_param.pUi = NULL;
    stCall_param.errorCode = 0;
    PARAM_SESSION_IN param;
    param.callback_param = &stCall_param;
    param.isBlock = BLOCKED;

    switch (appInfo.desktopType)
    {
    case VIRTUALAPP:
    case NORMALDESKTOP:
    {
        param_openChannel.str_ip = appInfo.hostname;
        break;
    }
    case DESKTOPPOOL:
    {
        CHECK_DESK_STATE_DATA st_checkResult;
        m_pSession->checkDesktopPoolState(param, appInfo.uuid, &st_checkResult);
        if(0==stCall_param.errorCode && st_checkResult.strIp.size()>0)
        {
            param_openChannel.str_ip = st_checkResult.strIp;
        }
        break;
    }
    case REMOTEDESKTOP:
    {
        CHECK_DESK_STATE_DATA st_checkResult;
        m_pSession->checkRemoteDesktopState(param, appInfo.uuid, &st_checkResult);
        if(0==stCall_param.errorCode && st_checkResult.strIp.size()>0)
        {
            param_openChannel.str_ip = st_checkResult.strIp;
        }
        break;
    }
    default:
    {
        LOG_ERR("unknown desktop type:%d", appInfo.desktopType);
        stCall_param.errorCode = -1;
        break;
    }
    }
    //m_hdSlotFusb = CreateFile(TEXT(MAIL_SLOT_NAME_FUSB), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    m_desktopIp = param_openChannel.str_ip;
    if(stCall_param.errorCode < 0)
    {
        LOG_ERR("checkDesktopPoolState failed, return value:%d", stCall_param);
        return stCall_param.errorCode;
    }

    CHANNEL_OP_DATA stChannelData;
    if(NULL == m_pSession)
        m_pSession = CSession::GetInstance();
    m_pSession->openChannel(param, param_openChannel, &stChannelData);
    if(stCall_param.errorCode < 0)
    {
        LOG_ERR("openChannel failed. return value:", stCall_param.errorCode);
        return stCall_param.errorCode;
    }
    std::string strData = std::string("--usbip ");
    std::string strFlag;
    if(strstr(resParam.usb, "####") != NULL)
    {
        strData = strData + "-- ";
        strFlag = " --flag 1 ";
    }
    else
    {
        strData = strData + resParam.usb + "-- ";
        strFlag = " --flag 0 ";
    }
    std::string strHost = "--port ";
    if(0 == stChannelData.str_ip.size() || stChannelData.str_ip == "127.0.0.1")
    {
        char chPort[MIN_LEN];
        memset(chPort, 0, MIN_LEN);
        //chPort[0] = '\0';
        sprintf(chPort, "%d", stChannelData.iPort); //old
        strHost = strHost + std::string(chPort) + " " + netWork.stPresentServer.serverAddress;//netWork.presentServer;
    }
    else
    {
        //strHost = strHost + "3240 " + stChannelData.str_ip;
        strHost = strHost + resData.stResInfo.port + " " + resData.stResInfo.ipAddr;
    }

    /*2014-10-30, haproxy转发模式下为tcp-usbip进程添加--uuid参数*/

    if (strlen(stChannelData.usb_Uuid) > 0)
        strData = strData + " --uuid " + stChannelData.usb_Uuid;

    std::string launchParams = string(APP_FUSB_TCP_PATH) + " " + strData + strFlag + strHost;

    m_pProcOp_usbMap = new CProcessOp(APP_FUSB_TCP_PATH, launchParams);
    if(NULL == m_pProcOp_usbMap)
    {
        LOG_ERR("%s", "NULL==m_pProcOp_usbMap");
        return -10;
    }
    iRet = m_pProcOp_usbMap->create();
//    VCLIENT_THREAD_CREATE(waitThread, NULL, waitUsbMap, m_pProcOp_usbMap);
//    THREAD_DETACH(waitThread);
    return iRet;
}

int CUsbMap::vaccessCloseChannel()
{
    int iRet = 0;
    if(m_desktopIp.size() <= 0)
    {
        return -1;
    }

    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI stCall_param;
    stCall_param.errorCode = 0;
    PARAM_SESSION_IN param;
    param.callback_param = &stCall_param;
    param.isBlock = BLOCKED;
//int shutdownRes(PARAM_SESSION_IN& st_param_in, char* pResTicket);
    if(NULL == m_pSession)
    {
        LOG_ERR("%s", "NULL==m_pSession");
        return -5;
    }
    PARAM_CHANNELOP_IN stChannelIn;
    CHANNEL_OP_DATA stChannelData;
    stChannelIn.str_ip = m_desktopIp;
    iRet = m_pSession->closeChannel(param, stChannelIn, &stChannelData);
    if(iRet < 0)
    {
        LOG_ERR("shutdownRes failed, return value:", iRet);
        return iRet;
    }
    if(stCall_param.errorCode < 0)
    {
        LOG_ERR("checkDesktopPoolState failed, return value:%d", stCall_param);
        return stCall_param.errorCode;
    }
    return 0;
}

void *CUsbMap::waitUsbMap(void *param)
{
    CProcessOp* pProcOp_usbMap = (CProcessOp *)param;
    if(NULL != pProcOp_usbMap)
        pProcOp_usbMap->wait();
    return 0;
}

int CUsbMap::quit()
{
    int iRet = 0;
    vaccessCloseChannel();
    if(NULL != m_pProcOp_usbMap)
    {
        iRet = m_pProcOp_usbMap->termate();
        waitUsbMap(m_pProcOp_usbMap);
        LOG_INFO("terminate USB process return value:%d", iRet);
    }
    return iRet;
}


#endif
