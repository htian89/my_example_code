#ifdef _WIN32
#include <windows.h>
#endif

#include "cfrontendbase.h"
#include "../common/cprocessop.h"
#include "../backend/csession.h"
#include "cusbmap.h"
#include "../common/errorcode.h"
#include "../common/log.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QDebug>
#include "ipc/ipcitalc.h"

extern IpcItalc *g_ipcItalc;

CFrontEndBase::CFrontEndBase(const LAUNCH_CALLBACK_INFO callBackInfo)
{
    m_pProcOp = NULL;
    m_pSession = CSession::GetInstance();
    m_pCallBackInfo = callBackInfo;
    m_bProcessIsRunning = false;
    m_bNeedMapUsb = false;
    m_bNeedMapFileSystem = false;
    m_bUserRequestQuit = false;
    m_hasReleased = false;
    m_event.setUnSignaled();//m_event.setSignaled();
    m_eventWaitRelease.setUnSignaled();
}
int CFrontEndBase::runInSubProcAndWait(string appName, string cmdParam, APP_LIST appInfo)
{
    if(m_bUserRequestQuit )
    {
        LOG_ERR("%s","m_bUserRequestQuit == true, does not going to create process.");
        return 0;
    }
    int iRet = 0;
    m_bProcessIsRunning = false;
    if(NULL != m_pProcOp)
    {
        m_event.setSignaled();
        quitProc();
        m_event.setUnSignaled();
        m_bUserRequestQuit = false;
        delete m_pProcOp;
        m_pProcOp = NULL;
    }

    if(m_bUserRequestQuit )
    {
        LOG_ERR("%s","m_bUserRequestQuit == true, does not going to create process.");
        return 0;
    }
    else
    {
        m_pProcOp = new CProcessOp(appName, cmdParam);
        iRet = m_pProcOp->create();
        if(NULL != m_pCallBackInfo.pFun)
        {
            m_pCallBackInfo.pFun(TYPE_LAUNCHAPP_CREATE_PROC_FINISHED, iRet, (void*)(&m_pCallBackInfo), (void*)(&appInfo));
        }
    }
    m_bProcessIsRunning = true;
//map usb
#ifdef WIN32
    CUsbMap UsbMap;
    //if(m_bNeedMapUsb)
    {
        if(NULL == m_pSession)
            m_pSession =CSession::GetInstance();
        UsbMap.launchUsbMap(appInfo, m_resParam, m_pSession->getNetwork(), m_resData);
    }
#else
   // m_usbMap = new CUsbMap;
   CUsbMap UsbMap;
    if(strstr(appName.c_str(), "frdp")!=NULL)//if(m_bNeedMapUsb && strstr(appName.c_str(), "frdp")!=NULL)
    {
        if(NULL == m_pSession)
            m_pSession =CSession::GetInstance();
        UsbMap.launchUsbMap(appInfo, m_resParam, m_pSession->getNetwork(), m_resData);
    }
#endif

//wait proc quit
    if(m_bUserRequestQuit)
    {
        LOG_ERR("%s","m_bUserRequestQuit == true, going to terminate process.");
        m_pProcOp->termate();
    }
    else
        m_pProcOp->wait();
    m_bProcessIsRunning =false;
    iRet = m_pProcOp->getExitCode();
    LOG_ERR("exit code:%d", iRet);
    if(NULL != m_pCallBackInfo.pFun)
    {
        m_pCallBackInfo.pFun(TYPE_LAUNCH_WAITPROC_RETURNED, iRet, (void*)(&m_pCallBackInfo), (void*)(&appInfo));
    }

    return iRet;
}

CFrontEndBase::~CFrontEndBase()
{
    cerr << "CFrontBase destruction!" << endl;
    if(g_ipcItalc != NULL)
    {
        cout << "CFrontEndBase   delete g_ipcItalc:  ###########" << endl;
        delete g_ipcItalc;
        g_ipcItalc = NULL;
    }
    release();
    m_eventWaitRelease.wait();
}

int CFrontEndBase::queryVaccess(const APP_LIST &appInfo)
{
    int iRet = 0;

    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI stCall_param;
    stCall_param.pUi = NULL;
    stCall_param.errorCode = 0;
    PARAM_SESSION_IN param;
    param.callback_param = &stCall_param;
    param.callbackFun = NULL;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
////check desk state
//    CHECK_DESK_STATE_DATA st_checkResult;
//    if(DESKTOPPOOL == appInfo.desktopType)
//    {
//        m_pSession->checkDesktopPoolState(param, appInfo.uuid, &st_checkResult);
//    }
//    else if(REMOTEDESKTOP == appInfo.desktopType)
//    {
//        m_pSession->checkRemoteDesktopState(param, appInfo.uuid, &st_checkResult);
//    }
//    if(stCall_param.errorCode < 0)
//    {
//        LOG_ERR("checkDesktopPoolState failed, return value:%d", stCall_param);
//        return stCall_param.errorCode;
//    }
//get resource parameters
    iRet = m_pSession->getResParam(appInfo.uuid, appInfo.desktopType, &m_resParam);
    if(iRet < 0)
    {
        LOG_ERR("getResParam failed, return value:", iRet);
        return iRet;
    }
//launch res
    stCall_param.errorCode = 0;
    PARAM_LAUNCH_RES_IN stLaunchResIn;
    stLaunchResIn.strResUuid = appInfo.uuid;
    stLaunchResIn.iResType = appInfo.desktopType;
    stLaunchResIn.iDisplayProtocol = appInfo.displayprotocol;
    m_pSession->launchResource(param, stLaunchResIn, &m_resData);
    if(stCall_param.errorCode < 0)
    {
        LOG_ERR("launchResource failed, return value:%d", stCall_param.errorCode);
        return stCall_param.errorCode;
    }
    //std::cout<<"launch res ticket:"<<m_resData.stResInfo.resourceTicket<<endl;
/*//attach disk
    if(bAttachDisk)
    {
        stCall_param.errorCode = 0;
        ATTACH_VDISK_DATA stAttachDiskData;
        m_pSession->attachVDisk(param, appInfo.uuid, &stAttachDiskData);
        if(stCall_param.errorCode < 0)
        {
            LOG_ERR("attachVDisk failed, return value:", stCall_param.errorCode);
            return stCall_param.errorCode;
        }

        iRet = 0;
        int count = 0;
        ASYN_QUERY_DATA stAsynQueryData;
        m_pSession->queryAsynJobResult(param, stAttachDiskData.strJobId.c_str(), &stAsynQueryData);
        while (0 == param.callback_param->errorCode)
        {//asyn query...
            if(count >= 40)
            {
                iRet = ERROR_UNABLE_TO_ATTACH;
                break;
            }
            m_pSession->queryAsynJobResult(param, stAttachDiskData.strJobId.c_str(), &stAsynQueryData);
            iRet = param.callback_param->errorCode;
            if(2 == iRet || -1 == iRet || 1 == iRet)
                break;
            Sleep(1000);
            count++;
        }
        if(iRet < 0)//attach disk failed
        {
            vaccessShutDownResource();
        }

    }*/
    return iRet;
}

int CFrontEndBase::release()
{
    m_mutex.lock();/////////
    if(!m_hasReleased)
    {
        m_hasReleased = true;        
    }
    else
    {
        m_mutex.unlock();
        return 0;
    }
    m_hasReleased = true;
    m_pCallBackInfo.pFun = NULL;
    m_pCallBackInfo.pFunUi = NULL;
    //in condition:the network speed is very slow and hasnot create the proc,
    //then destructor is called. the proc may crash because the CFrontEndBase has
    //destruct while the launch is not returned(because no call to m_event.wait();)
    cout << "CFrontEndbase::quitproc" << endl;
    quitProc();
    if(NULL != m_pProcOp)
    {
        delete m_pProcOp;
        m_pProcOp = NULL;
    }
//    if(NULL != m_usbMap)
//    {
//        delete m_usbMap;
//        m_usbMap = NULL;
//    }
    m_mutex.unlock();
    m_eventWaitRelease.setSignaled();
    return 0;
}

int CFrontEndBase::quitProc()
{
    int iRet = 0;
    m_bUserRequestQuit = true;
    LOG_INFO( "quit has called. m_pProcOp!=NULL:%d", NULL != m_pProcOp?1: 0);
    //vaccessShutDownResource();
    if(NULL != m_pProcOp)
    {
        iRet = m_pProcOp->termate();
        LOG_INFO("terminate process return value:%d", iRet);
    }
    m_event.wait();
    return iRet;
}
int CFrontEndBase::procStatus()
{
    if(NULL == m_pProcOp)
    {
        LOG_ERR("%s","NULL == m_pProcOp");
        return -1;
    }
    if(m_pProcOp->procQuit())
    {
        LOG_ERR("%s", "proc has quit....");
        return -5;
    }
    return 0;
}

int CFrontEndBase::AttachVDisk(const APP_LIST &appInfo, bool bAttach, const VIRTUALDISK& vDisk)
{
    int iRet = 0;
    if(bAttach)
    {
        if(!m_bUserRequestQuit)
            m_pCallBackInfo.pFun(TYPE_BEGIN_ATTACH_DISK, 0, (void*)(&m_pCallBackInfo), (void*)(&appInfo));
        iRet = vaccessAttachDisk(appInfo, vDisk);
        if(iRet<0 || iRet==2)
        {
            if(!m_bUserRequestQuit)
                m_pCallBackInfo.pFun(TYPE_LAUNCH_ATTACH_DISK_FAILED, iRet, (void*)(&m_pCallBackInfo), (void*)(&appInfo));
        }
        else
        {
            if(!m_bUserRequestQuit)
                m_pCallBackInfo.pFun(TYPE_LAUNCH_ATTACH_DISK_SUCCEED, iRet, (void*)(&m_pCallBackInfo), (void*)(&appInfo));
        }
    }
    return iRet;
}

int CFrontEndBase::vaccessAttachDisk(const APP_LIST &appInfo, const VIRTUALDISK& vDisk)
{
    CALLBACK_PARAM_UI stCall_param;
    stCall_param.pUi = NULL;
    stCall_param.errorCode = 0;
    PARAM_SESSION_IN param;
    param.callback_param = &stCall_param;
    param.callbackFun = NULL;
    param.isBlock = BLOCKED;
    stCall_param.errorCode = 0;
    ATTACH_VDISK_DATA stAttachDiskData;
    m_pSession->attachVDisk(param, appInfo.uuid, (int)(appInfo.desktopType), vDisk, &stAttachDiskData);
    if(stCall_param.errorCode < 0)
    {
        LOG_ERR("attachVDisk failed, return value:%d", stCall_param.errorCode);
        return stCall_param.errorCode;
    }

    int iRet = 0, count = 0;
    ASYN_QUERY_DATA stAsynQueryData;
    m_pSession->queryAsynJobResult(param, stAttachDiskData.strJobId.c_str(), &stAsynQueryData);
    while (0 == param.callback_param->errorCode)
    {//asyn query...
//        if(NULL!=m_pProcOp && m_pProcOp->procQuit())
//        {
//            iRet = 0;
//            LOG_INFO("%s", "subproc(desktop) has terminated.stop query...");
//            break;
//        }

//        if(count >= 180)
//        {
//            iRet = ERROR_UNABLE_TO_ATTACH;
//            break;
//        }
        if(m_bUserRequestQuit)
        {
            LOG_INFO("%s","user request quit...");
            iRet = 0;
            break;
        }
        m_pSession->queryAsynJobResult(param, stAttachDiskData.strJobId.c_str(), &stAsynQueryData);
        if(param.callback_param->errorCode>=0)
            iRet = stAsynQueryData.iJobStatus;//param.callback_param->errorCode;
        else
        {
            iRet = param.callback_param->errorCode;
            LOG_ERR("attach disk asyn query failed. return value:%d", iRet);
            continue;
        }
        if(2 == iRet || -1 == iRet || 1 == iRet)
        {
            LOG_ERR("attach disk asyn query result:%d", iRet);
            break;
        }
        if(m_bUserRequestQuit)
        {
            LOG_INFO("%s","user request quit...");
            iRet = 0;
            break;
        }
//        if(false == m_pProcOp->procQuit())
//        {
//            iRet = 0;
//            break;
//        }
        Sleep(1500);
        count++;
    }
    return iRet;
}

int CFrontEndBase::vaccessShutDownResource(int isRelease)
{
    int iRet = 0;

    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI stCall_param;
    stCall_param.pUi = NULL;
    stCall_param.errorCode = 0;
    PARAM_SESSION_IN param;
    param.callback_param = &stCall_param;
    param.callbackFun = NULL;
    param.isBlock = BLOCKED;
    param.taskUuid = taskUuid;
//int shutdownRes(PARAM_SESSION_IN& st_param_in, char* pResTicket);
    std::cout<<"shutdownRes res ticket:"<<m_resData.stResInfo.resourceTicket<<endl;
    iRet = m_pSession->shutdownRes(param, m_resData.stResInfo.resourceTicket, isRelease);
    if(iRet < 0)
    {
        LOG_ERR("shutdownRes failed, return value:", iRet);
        return iRet;
    }
    if(stCall_param.errorCode < 0)
    {
        LOG_ERR("shutdownRes failed, return value:%d", stCall_param.errorCode);
        return stCall_param.errorCode;
    }
    return 0;
}

int CFrontEndBase::vaccessListUserResource(LIST_USER_RESOURCE_DATA& userResData)
{
    int iRet = 0;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI stCall_param;
    stCall_param.pUi = NULL;
    stCall_param.errorCode = 0;
    PARAM_SESSION_IN param;
    param.callback_param = &stCall_param;
    param.callbackFun = NULL;
    param.isBlock = BLOCKED;
//int listUserResource(PARAM_SESSION_IN& st_param_in, bool bGetResParam = true, LIST_USER_RESOURCE_DATA* pRes= NULL);
    iRet = m_pSession->listUserResource(param, false, &userResData);
    if(iRet < 0)
    {
        LOG_ERR("listUserResource failed, return value:", iRet);
        return iRet;
    }
    if(stCall_param.errorCode < 0)
    {
        LOG_ERR("listUserResource failed, return value:%d", stCall_param.errorCode);
        return stCall_param.errorCode;
    }
    return 0;
}

int CFrontEndBase::setForeground(LAUNCH_TYPE type, const char *titleName )
{
    if( titleName != NULL){
        LOG_INFO("launchtype: %d, titlename: %s", type, titleName);
    }
#ifdef _WIN32
    if(NULL == m_pProcOp)
    {
        LOG_ERR("%s", "NULL == m_pProcOp");
        return -1;
    }

    //Because it is unable to find the correct fap window handle by using
    //GetTopWindow function, so now  it can only find the handle through fap's class name,
    //it may be a bug, so the function should do some change in the furture.
    if(type == LAUNCH_TYPE_FAP)
    {
        WCHAR wtitle[32] ;
        memset(wtitle, 0, sizeof(wtitle));
        int coverLen = MultiByteToWideChar(CP_UTF8, 0, titleName, -1, wtitle, 32);
        std::cout << "cover_len: " << coverLen << std::endl;
        WCHAR wclass[32] = TEXT("redc_wclass");
        HWND hh= FindWindow(wclass,  wtitle);
        SetForegroundWindow(hh);
        ShowWindow(hh, SW_NORMAL );
        return 0;
    }

    DWORD dwProcId = m_pProcOp->getProcId();
    if(0 == dwProcId)
    {
        LOG_ERR("%s", "get procid failed");
        return -5;
    }
    HWND h = GetTopWindow(NULL);
    while(h)
    {
        DWORD pid = 0;
        DWORD dwTheardId = GetWindowThreadProcessId(h,&pid);

        if(dwTheardId!=0 && pid==dwProcId)
        {
            SendMessage(h, WM_SYSCOMMAND, SC_RESTORE, 0);
            SetForegroundWindow(h);
            return 0;
        }
        h = GetNextWindow(h , GW_HWNDNEXT);
    }
    LOG_ERR("%s", "doesnot find window for process(id):%d", dwProcId);
    return -10;
#else
    return 0;
#endif
}

char *CFrontEndBase::getCmdSystem(const char *cmd)
{
#ifdef unix
    if(cmd == NULL)
        return NULL;
    char *tRet = (char *)malloc(4096);
    FILE *fpRead;
    int len;

    fpRead = popen(cmd, "r");

    tRet[0] = '\0';
    char str[4096] = {0};
    while (fgets(str, 4096 - 1, fpRead) != NULL)
    {
        strcat(tRet, str);
        memset(str, 0, sizeof(str));
    }

    len = strlen(tRet);
    if (tRet[len - 1] == '\n' || tRet[len - 1] == '\r')
        tRet[len - 1] = '\0';

    if(fpRead != NULL)
        pclose(fpRead);

    return tRet;
#endif
}

