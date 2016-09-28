#include "claunchapp.h"
#include "cfrontendrdp.h"
#include "cfrontendfap.h"
#ifndef _WIN32
#include "cfrontendterminal.h"
#endif
#include "../common/log.h"
#include "../common/ds_vclient.h"
#include "../common/errorcode.h"
#include "../common/cevent.h"

#include "cmessagebox.h"

#include <iostream>

using namespace std;

static CEvent* gpEvent_wait = NULL;//used to make it have a interval time between start two desktop links
int launchApp(const LAUNCH_THREAD_INFO *pstLaunchThreadInfo);
int launchCallBack(int iEventType, int iVal, void* pvoid1, void* pvoid2);
int disConnectAppThreadFun(void* pVoid);

CLaunchApp::CLaunchApp()
{
    if(NULL == gpEvent_wait)
    {
        gpEvent_wait = new CEvent();
    }
    gpEvent_wait->setSignaled();
    LOG_INFO("%s","set gpEvent_wait to signaled state");
}

CLaunchApp::~CLaunchApp()
{
//    for(std::map<std::string, LAUNCH_THREAD_INFO>::iterator iter = m_vctApp.begin(); iter != m_vctApp.end(); iter++)
//    {
//        eraseFromMap(iter->first);
//    }
    LOG_INFO("destop number:%d", m_vctApp.size());
//    for(std::map<std::string, LAUNCH_THREAD_INFO>::iterator iter = m_vctApp.begin(); iter != m_vctApp.end();)
//    {
//        eraseFromMap((iter++)->first);//please reference to http://blog.csdn.net/fan6662000/article/details/4012517
//    }
    std::map<std::string, LAUNCH_THREAD_INFO>::iterator iter = m_vctApp.begin();
    while(iter != m_vctApp.end())
    {
        eraseFromMap(iter->first);
         m_mutex.lock();
        iter = m_vctApp.begin();
         m_mutex.unlock();
    }
    LOG_INFO("%s","~CLaunchApp finished");
}

int CLaunchApp::appExit(const std::string& str_uuid, LAUNCH_TYPE iLaunchType)
{
    int iExit = 0;
    LAUNCH_THREAD_INFO stLaunchThreadInfo;
    if(findFromMap(str_uuid, &stLaunchThreadInfo) >=0)
    {
        if(NULL != stLaunchThreadInfo.pApp && 1==stLaunchThreadInfo.iAppExited && stLaunchThreadInfo.type==iLaunchType)
        {
            iExit = 1;
        }
    }
    return iExit;
}

int CLaunchApp::launchRdp(const PARAM_LAUNCH_COMMON_IN &stLaunchCommon, const LAUCH_RDP &stLaunchRdp)
{
    LAUNCH_THREAD_INFO stLaunchThreadInfo;
    if(findFromMap(stLaunchRdp.stAppInfo.uuid, &stLaunchThreadInfo) >=0)
    {
        if(NULL != stLaunchThreadInfo.pApp)
        {
//            if( stLaunchRdp.isConnected && stLaunchThreadInfo.type == LAUNCH_TYPE_RDP )
//            {
//                CMessageBox::TipBox(QObject::tr("The desktop is Connected by RDP"));
//                return 0;
//            }
            if(0 == stLaunchThreadInfo.iAppExited)
            {
                if(stLaunchThreadInfo.type == LAUNCH_TYPE_RDP)
                {
                    ((CFrontEndRdp*)(stLaunchThreadInfo.pApp))->setForeground(LAUNCH_TYPE_RDP, stLaunchRdp.stAppInfo.name);
                }
                else
                {
                    if(NULL != stLaunchCommon.pFunUi)
                    {
                        stLaunchCommon.pCallbackUi_param->errorCode = ERROR_ANOTER_PROTOCOL_ALIVE;
                        LAUNCH_DESKTOP_DATA* pstLaunchDesktopData = new LAUNCH_DESKTOP_DATA;
                        pstLaunchDesktopData->strDesktopUuid = stLaunchRdp.stAppInfo.uuid;
                        pstLaunchDesktopData->iOpType = 0;
                        (*(stLaunchCommon.pFunUi))(stLaunchCommon.pCallbackUi_param, TYPE_FRONTEND_LAUNCHDESKTOP, pstLaunchDesktopData);
                    }
                }
                return 0;
            }
            else
            {
                eraseFromMap(stLaunchRdp.stAppInfo.uuid);
            }
        }
    }
//new(create) a CFrontEndRdp instance
    LAUNCH_CALLBACK_INFO callBackInfo;
    callBackInfo.pFunUi = stLaunchCommon.pFunUi;
    callBackInfo.pCallbackUi_param = stLaunchCommon.pCallbackUi_param;
    callBackInfo.pFun = &launchCallBack;
    callBackInfo.pLaunchApp = this;
    callBackInfo.type = LAUNCH_TYPE_RDP;
    CFrontEndRdp* pRdp = new CFrontEndRdp(stLaunchRdp, callBackInfo);
    if(NULL == pRdp)
    {
        LOG_ERR("%s", "NULL == pRdp");
        return -5;
    }
    if(BLOCKED == stLaunchCommon.isBlock)//run in block mode
        return pRdp->Launch();
//run it in Thread
    LAUNCH_THREAD_INFO* pstThreadInfo = new LAUNCH_THREAD_INFO;
    if(NULL == pstThreadInfo)
    {
        LOG_ERR("%s", "NULL == pstThreadInfo");
        return -10;
    }
    pstThreadInfo->pApp = pRdp;
    pstThreadInfo->type = LAUNCH_TYPE_RDP;
    pstThreadInfo->iAppExited = 0;
    //create Thread , rdp launch desktop
    int iRet = CThread::createThread((THREAD_HANDLE*)(&(pstThreadInfo->hd)), NULL, (FUN_THREAD)(&launchApp), (void*)pstThreadInfo);
    if(iRet >= 0)
        insertToMap(stLaunchRdp.stAppInfo.uuid, *pstThreadInfo);
    return iRet;
}

int CLaunchApp::launchFap(const PARAM_LAUNCH_COMMON_IN &stLaunchCommon, const LAUCH_FAP &stLaunchFap)
{
    LAUNCH_THREAD_INFO stLaunchThreadInfo;
    if(findFromMap(stLaunchFap.stAppInfo.uuid, &stLaunchThreadInfo) >=0)
    {
        if(NULL != stLaunchThreadInfo.pApp)
        {
//            if( stLaunchFap.isConnected && stLaunchThreadInfo.type == LAUNCH_TYPE_FAP )
//            {
//                CMessageBox::TipBox(QObject::tr("The desktop is Connected by FAP"));
//                return 0;
//            }
             if(0 ==stLaunchThreadInfo.iAppExited)
            {
                if(stLaunchThreadInfo.type == LAUNCH_TYPE_FAP)
                {
                    ((CFrontEndFap*)stLaunchThreadInfo.pApp)->setForeground(LAUNCH_TYPE_FAP, stLaunchFap.stAppInfo.name);
                }
                else
                {
                    if(NULL != stLaunchCommon.pFunUi)
                    {
                        stLaunchCommon.pCallbackUi_param->errorCode = ERROR_ANOTER_PROTOCOL_ALIVE;
                        LAUNCH_DESKTOP_DATA* pstLaunchDesktopData = new LAUNCH_DESKTOP_DATA;
                        pstLaunchDesktopData->strDesktopUuid = stLaunchFap.stAppInfo.uuid;
                        pstLaunchDesktopData->iOpType = 0;
                        (*(stLaunchCommon.pFunUi))(stLaunchCommon.pCallbackUi_param, TYPE_FRONTEND_LAUNCHDESKTOP, pstLaunchDesktopData);
                    }
                }
                return 0;
            }
            else
            {

                eraseFromMap(stLaunchFap.stAppInfo.uuid);
            }
        }
    }
    LAUNCH_CALLBACK_INFO callBackInfo;
    callBackInfo.pFunUi = stLaunchCommon.pFunUi;
    callBackInfo.pCallbackUi_param = stLaunchCommon.pCallbackUi_param;
    callBackInfo.pFun = &launchCallBack;
    callBackInfo.pLaunchApp = this;
    callBackInfo.type = LAUNCH_TYPE_FAP;
    CFrontEndFap* pFap = new CFrontEndFap(stLaunchFap, callBackInfo);
    if(NULL == pFap)
    {
        LOG_ERR("%s", "NULL == pFap");
        return -5;
    }
    if(BLOCKED == stLaunchCommon.isBlock)//run in block mode
        return pFap->Launch();
    LAUNCH_THREAD_INFO* pstThreadInfo = new LAUNCH_THREAD_INFO;
    if(NULL == pstThreadInfo)
    {
        LOG_ERR("%s", "NULL == pstThreadInfo");
        return -10;
    }
    pstThreadInfo->pApp = pFap;
    pstThreadInfo->type = LAUNCH_TYPE_FAP;
    pstThreadInfo->iAppExited = 0;
    int iRet = CThread::createThread((THREAD_HANDLE*)(&(pstThreadInfo->hd)), NULL, (FUN_THREAD)(&launchApp), (void*)pstThreadInfo);
    if(iRet >= 0)
        insertToMap(stLaunchFap.stAppInfo.uuid, *pstThreadInfo);
    return iRet;
}
#ifndef _WIN32
 int CLaunchApp::launchTerminal(const PARAM_LAUNCH_COMMON_IN& stLaunchCommon, const LAUNCH_TERMINAL& stLaunchTerminal)
 {

//     if (CFrontEndTerminal::getRunningCount() > 0) {
//              CMessageBox::TipBox(QObject::tr("Already has one terminal App runing"));

//              return
//     }


    LAUNCH_THREAD_INFO stLaunchThreadInfo;
    if(findFromMap(stLaunchTerminal.stAppInfo.uuid, &stLaunchThreadInfo) >=0)
    {
        if(NULL != stLaunchThreadInfo.pApp)
        {
            if(0 == stLaunchThreadInfo.iAppExited)
            {
                if(stLaunchThreadInfo.type == LAUNCH_TYPE_TERMINAL)
                {

                    ((CFrontEndTerminal*)(stLaunchThreadInfo.pApp))->setForeground(LAUNCH_TYPE_TERMINAL, stLaunchTerminal.stAppInfo.name);
                }
                else
                {
                    if(NULL != stLaunchCommon.pFunUi)
                    {
                        stLaunchCommon.pCallbackUi_param->errorCode = ERROR_ANOTER_PROTOCOL_ALIVE;
                        LAUNCH_DESKTOP_DATA* pstLaunchDesktopData = new LAUNCH_DESKTOP_DATA;
                        pstLaunchDesktopData->strDesktopUuid = stLaunchTerminal.stAppInfo.uuid;
                        pstLaunchDesktopData->iOpType = 0;
                        (*(stLaunchCommon.pFunUi))(stLaunchCommon.pCallbackUi_param, TYPE_FRONTEND_LAUNCHDESKTOP, pstLaunchDesktopData);
                    }
                }
                return 0;
            }
            else
            {

                eraseFromMap(stLaunchTerminal.stAppInfo.uuid);
            }
        }
    }
//new(create) a CFrontEndTerminal instance
    LAUNCH_CALLBACK_INFO callBackInfo;
    callBackInfo.pFunUi = stLaunchCommon.pFunUi;
    callBackInfo.pCallbackUi_param = stLaunchCommon.pCallbackUi_param;
    callBackInfo.pFun = &launchCallBack;
    callBackInfo.pLaunchApp = this;
    callBackInfo.type = LAUNCH_TYPE_TERMINAL;
    CFrontEndTerminal* pTerminal = new CFrontEndTerminal(stLaunchTerminal, callBackInfo);
    if(NULL == pTerminal)
    {
        LOG_ERR("%s", "NULL == pTerminal");
        return -5;
    }
   if(BLOCKED == stLaunchCommon.isBlock) {//run in block mode

        return pTerminal->Launch();
   }


//run it in Thread
    LAUNCH_THREAD_INFO* pstThreadInfo = new LAUNCH_THREAD_INFO;
    if(NULL == pstThreadInfo)
    {
        LOG_ERR("%s", "NULL == pstThreadInfo");
        return -10;
    }
    pstThreadInfo->pApp = pTerminal;
    pstThreadInfo->type = LAUNCH_TYPE_TERMINAL;
    pstThreadInfo->iAppExited = 0;
    int iRet = CThread::createThread((THREAD_HANDLE*)(&(pstThreadInfo->hd)), NULL, (FUN_THREAD)(&launchApp), (void*)pstThreadInfo);
    if(iRet >= 0)
        insertToMap(stLaunchTerminal.stAppInfo.uuid, *pstThreadInfo);
    return iRet;
 }
#endif
int CLaunchApp::reloadVDiskInNewDesktop(const std::string uuid, RELOAD_VDISK_TYPE iReloadDisk, LAUNCH_TYPE iLaunchType)
{
    LAUNCH_THREAD_INFO launchThreadInfo;
    int iRet = findFromMap(uuid, &launchThreadInfo);
    if(iRet >= 0)
    {
        if(launchThreadInfo.iAppExited)
        {
            LOG_ERR("%s","thread has finished");
            iRet = -1;
        }
        else if(iLaunchType != launchThreadInfo.type)
        {
            LOG_ERR("launch type is not suitable. type:%d, in type:%d", (int)(launchThreadInfo.type), (int)(iLaunchType));
            iRet = -5;
        }
        else
        {
            if(iLaunchType == LAUNCH_TYPE_RDP)
            {
                CFrontEndRdp* pRdp = (CFrontEndRdp*)launchThreadInfo.pApp;
                pRdp->reloadVDiskInNewDesktop(iReloadDisk);
            }
            else if(iLaunchType == LAUNCH_TYPE_FAP)
            {
                CFrontEndFap* pFap = (CFrontEndFap*)launchThreadInfo.pApp;
                pFap->reloadVDiskInNewDesktop(iReloadDisk);
            }
            else
            {
                LOG_ERR("unknown launch type:%d", iLaunchType);
            }
        }
    }
    else
    {
        LOG_ERR(" find from map failed. uuid:%s", uuid.c_str());
    }
    return iRet;
}

int CLaunchApp::disconnectApp(const DISCONN_APP &stDisconnApp)
{
    DISCONN_APP_THREAD_INFO* pDissAppThreadInfo = new DISCONN_APP_THREAD_INFO;
    pDissAppThreadInfo->stDisconnApp = stDisconnApp;
    pDissAppThreadInfo->pLaunchApp = this;
    if(stDisconnApp.isBlock == BLOCKED)
    {
        disConnectAppThreadFun((void*)pDissAppThreadInfo);
        delete pDissAppThreadInfo;
        pDissAppThreadInfo = NULL;
        return 0;
    }
    else
    {
        CThread::createThread(NULL, NULL, (FUN_THREAD)(&disConnectAppThreadFun), (void*)pDissAppThreadInfo);
    }
    return 0;
}

int CLaunchApp::insertToMap(const std::string str, const LAUNCH_THREAD_INFO& launchInfo)
{
    m_mutex.lock();
    map<std::string, LAUNCH_THREAD_INFO>::iterator iter = m_vctApp.find(str);
    if(m_vctApp.end() != iter)
    {
        m_mutex.unlock();
        eraseFromMap(str);
        m_mutex.lock();
        LOG_ERR("error may be occured. desktop name:%s ", str.c_str());
    }
    m_vctApp.insert(pair<std::string, LAUNCH_THREAD_INFO>(str, launchInfo));
    LOG_INFO("insert to Map:%s", str.c_str());
    m_mutex.unlock();
    return 0;
}

int CLaunchApp::updateToMap(const std::string str, const LAUNCH_THREAD_INFO& launchInfo, bool bInsertIfNotFind/* = false*/)
{
    int iRet = 0;
    m_mutex.lock();
    map<std::string, LAUNCH_THREAD_INFO>::iterator iter = m_vctApp.find(str);
    if(m_vctApp.end() != iter)
    {
        iter->second = launchInfo;
        LOG_INFO("update to Map:%s", str.c_str());
    }
    else
    {
        LOG_ERR("not found. desktop name:%s ", str.c_str());
        if(bInsertIfNotFind)
        {
            m_vctApp.insert(pair<std::string, LAUNCH_THREAD_INFO>(str, launchInfo));
            LOG_INFO("insert to Map:%s", str.c_str());
        }
        else
        {
            iRet = -5;
        }
    }
    m_mutex.unlock();
    return iRet;
}

int CLaunchApp::eraseFromMap(const std::string str)
{
    CFrontEndBase* pApp = NULL;
    m_mutex.lock();
    map<std::string, LAUNCH_THREAD_INFO>::iterator iter = m_vctApp.find(str);
    if(m_vctApp.end() != iter)
    {
        pApp = iter->second.pApp;
        iter->second.pApp = NULL;
        m_vctApp.erase(iter);
        LOG_INFO("erase item:%s", str.c_str());
    }
    else
    {
        LOG_ERR("not found. desktop name:%s ", str.c_str());
    }
    m_mutex.unlock();
    if(NULL != pApp)
    {
        pApp->release();
        delete pApp;
    }
    return 0;
}

int CLaunchApp::findFromMap(const std::string str, LAUNCH_THREAD_INFO *pInfo)
{
    int iRet = 0;
    m_mutex.lock();
    map<std::string, LAUNCH_THREAD_INFO>::iterator iter = m_vctApp.find(str);
    if(m_vctApp.end() != iter)
    {
        if(NULL != pInfo)
            *pInfo = iter->second;
        else
        {
            iRet = -1;
            LOG_ERR("%s","NULL == pLaunchInfo");
        }
    }
    else
    {
        LOG_ERR("not found. desktop name:%s ", str.c_str());
        iRet = -5;
    }
    m_mutex.unlock();
    return iRet;
}

int launchApp(const LAUNCH_THREAD_INFO* pstLaunchThreadInfo)
{
    if(NULL == pstLaunchThreadInfo)
    {
        LOG_ERR("%s", "NULL == pstLaunchThreadInfo");
        return -1;
    }
    if(NULL != gpEvent_wait)
    {
        LOG_INFO("%s","gpEvent_wait going to wait...");
        gpEvent_wait->wait(2000);
        gpEvent_wait->setUnSignaled();
        LOG_INFO("%s","gpEvent_wait going to wait returned");
    }
    switch(pstLaunchThreadInfo->type)
    {
    case LAUNCH_TYPE_RDP:
    {
        CFrontEndRdp* pRdp = (CFrontEndRdp*)pstLaunchThreadInfo->pApp;
        if(NULL == pRdp)
        {
            LOG_ERR("%s", "NULL == pRdp");
            return -5;
        }
        pRdp->Launch();
        break;
    }
    case LAUNCH_TYPE_FAP:
    {
        CFrontEndFap* pFap = (CFrontEndFap*)(pstLaunchThreadInfo->pApp);
        if(NULL == pFap)
        {
            LOG_ERR("%s", "NULL == pFap");
            return -5;
        }
        pFap->Launch();
        break;
    }
#ifndef _WIN32
    case LAUNCH_TYPE_TERMINAL:
    {
        CFrontEndTerminal * pTerminal = (CFrontEndTerminal *)(pstLaunchThreadInfo->pApp);
        if(NULL == pTerminal)
        {
            LOG_ERR("%s", "NULL == pTerminal");
            return -5;
        }
        pTerminal->Launch();
        break;
    }
#endif
    default:
    {
        LOG_ERR("unknown type:%d", pstLaunchThreadInfo->type);
        break;
    }
    }
    delete pstLaunchThreadInfo;
    pstLaunchThreadInfo = NULL;
    return 0;
}

int disConnectAppThreadFun(void* pVoid)
{
    DISCONN_APP_THREAD_INFO* pDisconnAppThreadInfo = (DISCONN_APP_THREAD_INFO*)pVoid;
    if(NULL==pDisconnAppThreadInfo)
    {
        LOG_ERR("%s", "NULL==pDisconnAppThreadInfo");
        return 0;
    }
    if(NULL==pDisconnAppThreadInfo->pLaunchApp)
    {
        LOG_ERR("%s","NULL==pDisconnAppThreadInfo->pLaunchApp");
        return 0;
    }

    LAUNCH_THREAD_INFO launchThreadInfo;
    int iRet = pDisconnAppThreadInfo->pLaunchApp->findFromMap(pDisconnAppThreadInfo->stDisconnApp.strDesktopUuid, &launchThreadInfo);
    if(iRet >= 0)
    {
        if(launchThreadInfo.iAppExited)
        {
            LOG_ERR("%s","thread has finished");
            iRet = 0;
        }
        if(NULL != launchThreadInfo.pApp)
            launchThreadInfo.pApp->release();
        iRet = pDisconnAppThreadInfo->pLaunchApp->eraseFromMap(pDisconnAppThreadInfo->stDisconnApp.strDesktopUuid);
    }
    else
        LOG_ERR(" find from map failed. uuid:%s", pDisconnAppThreadInfo->stDisconnApp.strDesktopUuid.c_str());

    if(pDisconnAppThreadInfo->stDisconnApp.isBlock==UNBLOCK && NULL!=pDisconnAppThreadInfo->stDisconnApp.pFunUi)
    {
        DISCONNECT_APP_DATA* pData = new DISCONNECT_APP_DATA;
        pData->strDesktopUuid = pDisconnAppThreadInfo->stDisconnApp.strDesktopUuid;
        pDisconnAppThreadInfo->stDisconnApp.pFunUi(pDisconnAppThreadInfo->stDisconnApp.pCallbackUi_param, TYPE_FRONTEND_DISCONN_APP, pData);
    }

    return 0;
}

int dealCallBack(LAUNCH_CALLBACK_INFO* pCallBackInfo, APP_LIST* pAppInfo, int iVal, int iEventType)
{
    if(NULL == pCallBackInfo || NULL == pAppInfo)
    {
        LOG_ERR("%s","(NULL == pCallBackInfo || NULL == pAppInfo");
        return -1;
    }
    if(NULL != pCallBackInfo->pLaunchApp)
    {
        LAUNCH_THREAD_INFO stThreadInfo;
        int iRet = pCallBackInfo->pLaunchApp->findFromMap(pAppInfo->uuid, &stThreadInfo);
        if(iRet >= 0)
        {
            stThreadInfo.iAppExited = 1;
            pCallBackInfo->pLaunchApp->updateToMap(pAppInfo->uuid, stThreadInfo);
        }
    }
    if(NULL != pCallBackInfo->pFunUi)
    {
        if(NULL != pCallBackInfo->pCallbackUi_param)
            pCallBackInfo->pCallbackUi_param->errorCode = iVal;
        LAUNCH_DESKTOP_DATA* pstLaunchDesktopData = new LAUNCH_DESKTOP_DATA;
        pstLaunchDesktopData->strDesktopUuid = pAppInfo->uuid;
        pstLaunchDesktopData->iOpType = iEventType;
        pstLaunchDesktopData->stAppInfo = *pAppInfo;
        (*(pCallBackInfo->pFunUi))(pCallBackInfo->pCallbackUi_param, TYPE_FRONTEND_LAUNCHDESKTOP, pstLaunchDesktopData);//(int)pCallBackInfo->type
    }
    return 0;
}

int launchCallBack(int iEventType, int iVal, void* pvoid1, void* pvoid2)
{
    LAUNCH_CALLBACK_INFO* pCallBackInfo = (LAUNCH_CALLBACK_INFO*)pvoid1;
    if(NULL == pCallBackInfo)
    {
        LOG_ERR("%s","NULL == pCallBackInfo");
    }

    switch(pCallBackInfo->type)
    {
    case LAUNCH_TYPE_RDP:
    {
        switch(iEventType)
        {
        case TYPE_LAUNCHAPP_CREATE_PROC_FINISHED:
        case TYPE_LAUNCH_WAITPROC_RETURNED:
        {
            LOG_INFO("LAUNCH_TYPE_RDP\teventtype:%d return value:%d deskname:%s",\
                     iEventType, iVal, NULL==pvoid1 ? "":((APP_LIST*)pvoid2)->name);

            if(NULL!=gpEvent_wait && TYPE_LAUNCHAPP_CREATE_PROC_FINISHED==iEventType)
            {
                gpEvent_wait->clear();
                gpEvent_wait->setSignaled();
                LOG_INFO("%s","set gpEvent_wait to signaled state");
            }

            break;
        }
        case TYPE_QUERYVACCESS_FAILED:
        case TYPE_LAUNCH_FORM_CMDPARAM_FAILED:
        case TYPE_LAUNCH_SHUTDOWN_RES_FINISHED:
        case TYPE_DESKTOP_NOT_AVAILABLE:
        {
            LOG_INFO("LAUNCH_TYPE_RDP\teventtype:%d return value:%d deskname:%s",\
                     iEventType, iVal, NULL==pvoid2 ? "":((APP_LIST*)pvoid2)->name);
            dealCallBack(pCallBackInfo, (APP_LIST*)pvoid2, iVal, iEventType);
            break;
        }
        case TYPE_LAUNCH_ATTACH_DISK_FAILED:
        case TYPE_LAUNCH_ATTACH_DISK_SUCCEED:
        case TYPE_BEGIN_ATTACH_DISK:
        {
            if(NULL != pCallBackInfo->pFunUi)
            {
                if(NULL != pCallBackInfo->pCallbackUi_param)
                    pCallBackInfo->pCallbackUi_param->errorCode = iVal;
                LAUNCH_DESKTOP_DATA* pstLaunchDesktopData = new LAUNCH_DESKTOP_DATA;
                APP_LIST* pAppList = (APP_LIST*)pvoid2;
                if(NULL != pAppList)
                {//on ui, it will be delete pParamUi, but we should use it in the later
                    CALLBACK_PARAM_UI* pParamUi = new CALLBACK_PARAM_UI;
                    *pParamUi = *(pCallBackInfo->pCallbackUi_param);
                    pstLaunchDesktopData->strDesktopUuid = pAppList->uuid;
                    pstLaunchDesktopData->iOpType = iEventType;
                    pstLaunchDesktopData->stAppInfo = *((APP_LIST*)pvoid2);
                    (*(pCallBackInfo->pFunUi))(pParamUi, TYPE_FRONTEND_LAUNCHDESKTOP, pstLaunchDesktopData);
                }
            }
            break;
        }
        default:
        {
            LOG_ERR("unknown type:%d", iEventType);
            break;
        }
        }
        break;
    }
    case LAUNCH_TYPE_FAP:
    {
        switch(iEventType)
        {
        case TYPE_LAUNCHAPP_CREATE_PROC_FINISHED:
        case TYPE_LAUNCH_WAITPROC_RETURNED:
        {
            LOG_INFO("LAUNCH_TYPE_FAP\teventtype:%d return value:%d deskname:%s",\
                     iEventType, iVal, NULL==pvoid2 ? "":((APP_LIST*)pvoid2)->name);
            if(NULL!=gpEvent_wait && TYPE_LAUNCHAPP_CREATE_PROC_FINISHED==iEventType)
            {
                gpEvent_wait->clear();
                gpEvent_wait->setSignaled();
                LOG_INFO("%s","set gpEvent_wait to signaled state");
            }
            break;
        }
        case TYPE_QUERYVACCESS_FAILED:
        case TYPE_LAUNCH_FORM_CMDPARAM_FAILED:
        case TYPE_LAUNCH_SHUTDOWN_RES_FINISHED:
        case TYPE_DESKTOP_NOT_AVAILABLE:
        {
            LOG_INFO("LAUNCH_TYPE_FAP\t%d return value:%d deskname:%s",\
                     iEventType, iVal, NULL==pvoid2 ? "":((APP_LIST*)pvoid2)->name);
            dealCallBack(pCallBackInfo, (APP_LIST*)pvoid2, iVal, iEventType);
            break;
        }
        case TYPE_LAUNCH_ATTACH_DISK_FAILED:
        case TYPE_LAUNCH_ATTACH_DISK_SUCCEED:
        case TYPE_BEGIN_ATTACH_DISK:
        {
            if(NULL != pCallBackInfo->pFunUi)
            {
                if(NULL != pCallBackInfo->pCallbackUi_param)
                    pCallBackInfo->pCallbackUi_param->errorCode = iVal;
                LAUNCH_DESKTOP_DATA* pstLaunchDesktopData = new LAUNCH_DESKTOP_DATA;
                APP_LIST* pAppList = (APP_LIST*)pvoid2;
                if(NULL != pAppList)
                {//on ui, it will delete pParamUi, but we should use it in the later
                    CALLBACK_PARAM_UI* pParamUi = new CALLBACK_PARAM_UI;
                    *pParamUi = *(pCallBackInfo->pCallbackUi_param);
                    pstLaunchDesktopData->strDesktopUuid = pAppList->uuid;
                    pstLaunchDesktopData->iOpType = iEventType;
                    pstLaunchDesktopData->stAppInfo = *((APP_LIST*)pvoid2);
                    (*(pCallBackInfo->pFunUi))(pParamUi, TYPE_FRONTEND_LAUNCHDESKTOP, pstLaunchDesktopData);
                }
            }
            break;
        }
        default:
        {
            LOG_ERR("unknown type:%d", iEventType);
            break;
        }
        }
        break;
    }
    case LAUNCH_TYPE_TERMINAL:
    {
        switch(iEventType)
        {
        case TYPE_LAUNCHAPP_CREATE_PROC_FINISHED:
        case TYPE_LAUNCH_WAITPROC_RETURNED:
        {
            LOG_INFO("LAUNCH_TYPE_TERMINAL\teventtype:%d return value:%d deskname:%s",\
                     iEventType, iVal, NULL==pvoid2 ? "":((APP_LIST*)pvoid2)->name);
            if(NULL!=gpEvent_wait && TYPE_LAUNCHAPP_CREATE_PROC_FINISHED==iEventType)
            {
                gpEvent_wait->clear();
                gpEvent_wait->setSignaled();
                LOG_INFO("%s","set gpEvent_wait to signaled state");
            }
            break;
        }
        case TYPE_QUERYVACCESS_FAILED:
        case TYPE_LAUNCH_FORM_CMDPARAM_FAILED:
        case TYPE_LAUNCH_SHUTDOWN_RES_FINISHED:
        case TYPE_DESKTOP_NOT_AVAILABLE:
        {
            LOG_INFO("LAUNCH_TYPE_TERMINAL\t%d return value:%d deskname:%s",\
                     iEventType, iVal, NULL==pvoid2 ? "":((APP_LIST*)pvoid2)->name);
            dealCallBack(pCallBackInfo, (APP_LIST*)pvoid2, iVal, iEventType);
            break;
        }
        case TYPE_LAUNCH_ATTACH_DISK_FAILED:
        case TYPE_LAUNCH_ATTACH_DISK_SUCCEED:
        case TYPE_BEGIN_ATTACH_DISK:
        {
            if(NULL != pCallBackInfo->pFunUi)
            {
                if(NULL != pCallBackInfo->pCallbackUi_param)
                    pCallBackInfo->pCallbackUi_param->errorCode = iVal;
                LAUNCH_DESKTOP_DATA* pstLaunchDesktopData = new LAUNCH_DESKTOP_DATA;
                APP_LIST* pAppList = (APP_LIST*)pvoid2;
                if(NULL != pAppList)
                {//on ui, it will delete pParamUi, but we should use it in the later
                    CALLBACK_PARAM_UI* pParamUi = new CALLBACK_PARAM_UI;
                    *pParamUi = *(pCallBackInfo->pCallbackUi_param);
                    pstLaunchDesktopData->strDesktopUuid = pAppList->uuid;
                    pstLaunchDesktopData->iOpType = iEventType;
                    pstLaunchDesktopData->stAppInfo = *((APP_LIST*)pvoid2);
                    (*(pCallBackInfo->pFunUi))(pParamUi, TYPE_FRONTEND_LAUNCHDESKTOP, pstLaunchDesktopData);
                }
            }
            break;
        }
        default:
        {
            LOG_ERR("unknown type:%d", iEventType);
            break;
        }
        }
        break;
    }

    default:
    {
        LOG_ERR("unknown LAUNCH_TYPE:%d", pCallBackInfo->type);
        break;
    }
    }

    return 0;
}
