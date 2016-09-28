#include <QString>
#include <stdio.h>
#include "../common/cprocessop.h"
#include "../common/common.h"
#include "../backend/cthreadpool.h"
#include "../common/log.h"
#include "cupdate.h"
#include "filepath.h"
#include <iostream>
#ifdef WIN32
#include <windows.h>
#include <winsvc.h>
#endif

//all of the following global variables are defined in main.cpp
extern QString strAppPath;
extern int g_log_writeFileLevel;
extern bool gb_showCmdWindow;
extern char g_log_file_path[512];

//*****************************************************************
//function Name:formParameter
//  forms cmd parameters for updatevclient.exe
//parameter:
//	stVClientSettings(const SETTINGS_VCLIENT) [in]:
//      contains information about vaccess info, update type
//	strPath(std::string&)[out]:
//      the parameters that formed.
//return value:
//      currently always return 0
//
//ATTENTION:
//      if the parameter is a path(such as c:\a\b\c), please donot add '\' or '/'
//  at the end of the path(et:donot use c:\a\b\c\), becauce the updatevclient.exe
//  cannot correctly analysis the parameter in this case
//*****************************************************************
int formParameter(const SETTINGS_VCLIENT stVClientSettings, std::string& strPath)
{
    LOG_INFO("working directory:%s",strAppPath.toUtf8().data());

    char caUpdateType[4], caLogLevel[4], caCurVersion[24];
    sprintf(caUpdateType, "%d", int(stVClientSettings.m_updateSetting));
    sprintf(caLogLevel, "%d", g_log_writeFileLevel);
    sprintf(caCurVersion, "%d.%d.%d.%d", VER1, VER2, VER3, VER4);

    strPath = strPath + "\"" + strAppPath.toUtf8().data() + "/" + UPDATE_VCLIENT_PATH +  + "\"";
    if(gb_showCmdWindow)
    {
        strPath += " -with-debug ";
    }

    char charArray[MAX_LEN];
    memset(charArray, 0, MAX_LEN);
    strcpy(charArray, strAppPath.toUtf8().data());
    size_t len = strlen(charArray);
    if(len>0)
    {
        if(charArray[len-1]=='\\'||charArray[len-1]=='/')
            charArray[len-1] = '\0';
        strPath = strPath + " --installpath \"" + charArray + "\"";
    }

    memset(charArray, 0, MAX_LEN);
    strcpy(charArray, updatePath.data());
    len = strlen(charArray);
    if(len>0)
    {
        if(charArray[len-1]=='\\'||charArray[len-1]=='/')
            charArray[len-1] = '\0';
        strPath = strPath + " --downloadpath \"" + charArray + "\"";
    }

    strPath = strPath + " --logpath \"" + g_log_file_path + "\"";
    strPath = strPath + " --updatetype " + caUpdateType;
    strPath = strPath + " --loglevel " + caLogLevel;
    strPath = strPath + " --currentversion " + caCurVersion;
    if(strlen(stVClientSettings.m_network.stFirstServer.serverAddress)>0)
    {
        if(stVClientSettings.m_network.stFirstServer.isHttps)
            strPath = strPath + " --preferenserver " + "https://";
        else
            strPath = strPath + " --preferenserver " + "http://";
        strPath =strPath + stVClientSettings.m_network.stFirstServer.serverAddress + ":" + stVClientSettings.m_network.stFirstServer.port;
    }
    if(strlen(stVClientSettings.m_network.stAlternateServer.serverAddress)>0)
    {
        if(stVClientSettings.m_network.stAlternateServer.isHttps)
            strPath = strPath + " --alternativeserver " + "https://";
        else
            strPath = strPath + " --alternativeserver " + "http://";
        strPath =strPath + stVClientSettings.m_network.stAlternateServer.serverAddress + ":" +stVClientSettings.m_network.stAlternateServer.port;
    }
//    if(strlen(stVClientSettings.m_network.firstServer)>0)
//    {
//        if(stVClientSettings.m_network.isHttps)
//            strPath = strPath + " --preferenserver " + "https://";
//        else
//            strPath = strPath + " --preferenserver " + "http://";
//        strPath =strPath + stVClientSettings.m_network.firstServer + ":" + stVClientSettings.m_network.port;
//    }
//    if(strlen(stVClientSettings.m_network.alternateServer)>0)
//    {
//        if(stVClientSettings.m_network.isHttps)
//            strPath = strPath + " --alternativeserver " + "https://";
//        else
//            strPath = strPath + " --alternativeserver " + "http://";
//        strPath =strPath + stVClientSettings.m_network.alternateServer + ":" +stVClientSettings.m_network.port;
//    }

    std::cout<<"updatevClient param:"<<strPath<<std::endl;
    LOG_INFO("updatevClient param:%s", strPath.c_str());
    return 0;
}

CUpdate::CUpdate(const SETTINGS_VCLIENT& vClientSetting)
{
    m_vclientSetting = vClientSetting;
    m_iRet = 0;
    m_pProc = NULL;
    m_hasFinished = false;
}

CUpdate::~CUpdate()
{
    if(NULL != m_pProc)
    {
        m_pProc->termate();
        while (!m_hasFinished)
            sleep(1);
        delete m_pProc;
        m_pProc = NULL;
    }
}

void CUpdate::run()
{
    std::string strPath;
    formParameter(m_vclientSetting, strPath);
    m_pProc = new CProcessOp("", strPath.c_str());
    m_iRet = m_pProc->create(strAppPath.toUtf8().data());
    if(m_iRet < 0)
    {
        LOG_ERR("create proce failed. return value:%d", m_iRet);
        return;
    }
    m_iRet = m_pProc->wait();
    if(m_iRet < 0)
    {
        LOG_ERR("wait proc failed. return value:%d", m_iRet);
        m_hasFinished = true;
        return;
    }
    m_hasFinished = true;
}

//*****************************************************************
//function Name:stopAutoUpdateService
//  stops updatevclient services(but do not uninstall it)
//parameter:
//
//return value:
//  >=0 success
//  <0  fail
//*****************************************************************
int CUpdate::stopAutoUpdateService()
{
#ifdef WIN32
    std::string str_cmd = " /c net.exe stop ";
    str_cmd += AUTO_UPDATE_VCLIENT_SERVICE_NAME;
    HINSTANCE hinst = ShellExecuteA(NULL, NULL, "cmd.exe", str_cmd.c_str(), NULL, SW_HIDE);
    if((int)hinst<=32)
    {
        LOG_ERR("execute %s failed. return value:%d", str_cmd.c_str(), hinst);
        return -1;
    }
#endif
    return 0;
}

//*****************************************************************
//function Name:setAutoUpdate
//  stops updatevclient services(but do not uninstall it)
//parameter:
//  bAutoUpdate(bool)[in]:
//      if bAutoUpdate is true,then install the updatevclient
//    service and start it, else stop the service and uninstall it.
//return value:
//  >=0 success
//  <0  fail
//*****************************************************************
int CUpdate::setAutoUpdate(bool bAutoUpdate)
{
    if(bAutoUpdate){
        LOG_INFO("%s","set auto update");
    }
#ifdef WIN32
    LOG_INFO("set AutoUpdate bAutoUpdate= %d", int(bAutoUpdate));
    saveUserPath();
    if(!bAutoUpdate)
    {//stop service
        stopAutoUpdateService();
    }
    //install(uninstall) service
    std::string cmd = AUTO_UPDATE_VCLIENT_SERVICE_APP;
    if(bAutoUpdate)
        cmd += " -i";
    else
        cmd += " -u";
    CProcessOp procOp("", cmd.c_str());
    int iRet = procOp.create(NULL, 0);
    if(iRet < 0)
    {
        LOG_ERR("create proc failed. return value:%d", iRet);
        return iRet;
    }
    iRet = procOp.wait();
    if(iRet < 0)
    {
        LOG_ERR("wait proc failed. return value:%d", iRet);
        return iRet;
    }
    iRet =  procOp.getExitCode();
    if(iRet != 0)
    {
        LOG_ERR("exit code is %d. failed", iRet);
        return -1;
    }

    if(bAutoUpdate)
    {//start service
        std::string str_cmd = "/c net.exe start ";
        str_cmd += AUTO_UPDATE_VCLIENT_SERVICE_NAME;
        HINSTANCE hinst = ShellExecuteA(NULL, NULL, "cmd.exe", str_cmd.c_str(), NULL, SW_HIDE);
        if((int)hinst<=32)
        {
            LOG_ERR("execute %s failed. return value:%d", str_cmd.c_str(), hinst);
        }
    }
    return 0;
#else
    return 0;
#endif
}

//*****************************************************************
//function Name:getAutoUpdateServiceStatus
//  query updatevclient services's start type
//parameter:
//
//return value:
//  >0  query type is SERVICE_AUTO_START
//  =0 query type is not SERVICE_AUTO_START
//  <0 query failed
//*****************************************************************
int CUpdate::getAutoUpdateServiceStatus()
{
#ifdef WIN32
//Get a handle to the SCM database.
    SC_HANDLE hdScManager = OpenSCManagerA(NULL, NULL, SERVICE_QUERY_CONFIG);
    if (NULL == hdScManager)
    {
        LOG_ERR("OpenSCManagerA failed. reason:%d", GetLastError());
        return -1;
    }
//Get a handle to the service.
    SC_HANDLE hdScService = OpenServiceA(hdScManager, AUTO_UPDATE_VCLIENT_SERVICE_NAME,\
                             SERVICE_QUERY_CONFIG);
    if (hdScService == NULL)
    {
        LOG_ERR("OpenService failed. reason:%d", GetLastError());
        CloseServiceHandle(hdScManager);
        return -5;
    }
// Get the configuration information.
    LPQUERY_SERVICE_CONFIGA lpsc = NULL;
    DWORD dwBytesNeeded = 0, cbBufSize = 0;
    if(!QueryServiceConfigA(hdScService, NULL, 0, &dwBytesNeeded))
    {
        DWORD dwError = GetLastError();
        if( ERROR_INSUFFICIENT_BUFFER == dwError )
        {
            cbBufSize = dwBytesNeeded;
            lpsc = (LPQUERY_SERVICE_CONFIGA) LocalAlloc(LMEM_FIXED, cbBufSize);
        }
        else
        {
            LOG_ERR("QueryServiceConfig failed. reason: %d", dwError);
            CloseServiceHandle(hdScService);
            CloseServiceHandle(hdScManager);
            return -5;
        }
    }

    if(!QueryServiceConfigA(hdScService, lpsc, cbBufSize,&dwBytesNeeded))
    {
        LocalFree(lpsc);
        CloseServiceHandle(hdScService);
        CloseServiceHandle(hdScManager);
        LOG_ERR("QueryServiceConfig failed (%d)", GetLastError());
        return -10;
    }
    int iRet = 0;
    if(SERVICE_AUTO_START == lpsc->dwStartType)
        iRet = 1;
    else
        iRet = 0;

    LocalFree(lpsc);
    CloseServiceHandle(hdScService);
    CloseServiceHandle(hdScManager);
    return iRet;
#else
    return 0;
#endif
}


//int runUpdate(const SETTINGS_VCLIENT* pstVClientSettings)
//{
//    LOG_INFO("working directory:%s",strAppPath.toUtf8().data());
//    if(NULL == pstVClientSettings)
//    {
//        LOG_ERR("%s", "pstVClientSettings");
//        return -1;
//    }
//    SETTINGS_VCLIENT stVClientSettings = *pstVClientSettings;
//    char caUpdateType[4], caLogLevel[4], caCurVersion[24];
//    sprintf(caUpdateType, "%d", int(stVClientSettings.m_updateSetting));
//    sprintf(caLogLevel, "%d", g_log_writeFileLevel);
//    sprintf(caCurVersion, "%d.%d.%d.%d", VER1, VER2, VER3, VER4);
//    std::string strPath;
//    strPath = strPath + "\"" + strAppPath.toUtf8().data() + "/" + UPDATE_VCLIENT_PATH +  + "\"";
//    if(gb_showCmdWindow)
//    {
//        strPath += " -with-debug ";
//    }

//    char charArray[MAX_PATH];
//    memset(charArray, 0, MAX_PATH);
//    strcpy(charArray, strAppPath.toUtf8().data());
//    size_t len = strlen(charArray);
//    if(len>0)
//    {
//        if(charArray[len-1]=='\\'||charArray[len-1]=='/')
//            charArray[len-1] = '\0';
//        strPath = strPath + " --installpath \"" + charArray + "\"";
//    }

//    memset(charArray, 0, MAX_PATH);
//    strcpy(charArray, updatePath.data());
//    len = strlen(charArray);
//    if(len>0)
//    {
//        if(charArray[len-1]=='\\'||charArray[len-1]=='/')
//            charArray[len-1] = '\0';
//        strPath = strPath + " --downloadpath \"" + charArray + "\"";
//    }

//    strPath = strPath + " --logpath \"" + g_log_file_path + "\"";
//    strPath = strPath + " --updatetype " + caUpdateType;
//    strPath = strPath + " --loglevel " + caLogLevel;
//    strPath = strPath + " --currentversion " + caCurVersion;
//    if(strlen(stVClientSettings.m_network.firstServer)>0)
//    {
//        if(stVClientSettings.m_network.isHttps)
//            strPath = strPath + " --preferenserver " + "https://";
//        else
//            strPath = strPath + " --preferenserver " + "http://";
//        strPath =strPath + stVClientSettings.m_network.firstServer + ":" + stVClientSettings.m_network.port;
//    }
//    if(strlen(stVClientSettings.m_network.alternateServer)>0)
//    {
//        if(stVClientSettings.m_network.isHttps)
//            strPath = strPath + " --alternativeserver " + "https://";
//        else
//            strPath = strPath + " --alternativeserver " + "http://";
//        strPath =strPath + stVClientSettings.m_network.alternateServer + ":" +stVClientSettings.m_network.port;
//    }

//    std::cout<<"updatevClient param:"<<strPath<<std::endl;
//    LOG_INFO("updatevClient param:%s", strPath.c_str());

//    int iTryTimes = 0, iQueryTimes = 0;
//    while (true)
//    {
//        //proc.start(strPath.c_str());
//        CProcessOp procOp("", strPath.c_str());
//        procOp.create(strAppPath.toUtf8().data());
//        iQueryTimes = 0;
//        while(true)
//        {//query process status
//            Sleep(1000);
//            if(procOp.procQuit())
//            {//proc has quited
//                procOp.wait();
//                LOG_ERR("%s", "proc has quit...");
//            }
//            else
//                return 0;
//            iQueryTimes ++;
//            if(iQueryTimes >= 20)
//            {
//                LOG_ERR("%s", "process doesnot going into running state for a long time. may be error occured");
//            }
//        }
//        iTryTimes ++;
//        if(3 == iTryTimes)
//        {
//            LOG_ERR("%s","tryed to start 3 times. but all failed....");
//            return -1;
//        }
//    }
//    return 0;
//}
