#ifdef WIN32
#include <windows.h>
#endif

#include "csession.h"
#include "vaccess.h"
#include "../common/ds_access.h"
#include "../common/errorcode.h"
#include <iostream>
#include "string.h"


using namespace std;

CSession* volatile  CSession::m_pSessionInst = NULL;

CSession::CSession()
{
    memset(&m_stNetwork, 0, sizeof(NETWORK));
    memset(&m_stUserInfo, 0, sizeof(USER_INFO));
    memset(&m_accountInfo, 0, sizeof(NT_ACCOUNT_INFO));
    m_loginType = LOGIN_TYPE_UNKNOWN;
}

CSession::CSession(const NETWORK network, const USER_INFO userinfo)
{
    setNetwork(network);
    setUserInfo(userinfo);
    //m_iHasLogin = 0;
    memset(&m_accountInfo, 0, sizeof(NT_ACCOUNT_INFO));
}
CSession::~CSession()
{

}

CMutexOp CSession::m_mutex;

int CSession::cancelTask(const taskUUID taskUuid)
{
    return m_threadPool.changeThreadInfo(taskUuid);
}

int set_pSessionToNULL(void*pvoid, int iType)
{
    if(NULL == pvoid)
    {
        LOG_ERR("%s","pvoid == NULL");
        return 0;
    }
    switch(iType)
    {
    case TYPE_GETDOMAIN:
    {
        ST_ACCESS_GETDOMAIN_IN* param = (ST_ACCESS_GETDOMAIN_IN*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_GET_USERNAME:
    {
        ST_ACCESS_GET_USERNAME *param = (ST_ACCESS_GET_USERNAME*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_LOGIN:
    {
        ST_ACCESS_LOGIN_IN* param = (ST_ACCESS_LOGIN_IN*) pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_KEEPSESSION:
    {
        ST_ACCESS_KEEPSESSION_IN* param = (ST_ACCESS_KEEPSESSION_IN*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_LOGOUT:
    {
        ST_ACCESS_LOGOUT_IN* param = (ST_ACCESS_LOGOUT_IN*) pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_GETUSERINFO:
    {
        ST_ACCESS_GETUSERINFO_IN* param = (ST_ACCESS_GETUSERINFO_IN*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_GETICON:
    {
        ST_ACCESS_GETICON_IN* param = (ST_ACCESS_GETICON_IN*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_LIST_USER_RES:
    {
        ST_ACCESS_LISTRES_IN* param = (ST_ACCESS_LISTRES_IN*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_LAUNCH_RES:
    {
        ST_ACCESS_LAUNCHRES_IN* param = (ST_ACCESS_LAUNCHRES_IN*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_SHUTDOWN_RES:
    {
        ST_ACCESS_SHUTDOWN_RES_IN* param =(ST_ACCESS_SHUTDOWN_RES_IN*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_CHANGE_USER_INFO:
    {
        ST_ACCESS_CHANGE_USER_INFO_IN* param =(ST_ACCESS_CHANGE_USER_INFO_IN*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_CHANGE_NTUSER_INFO:
    {
        ST_ACCESS_CHANGE_USER_INFO_IN* param =(ST_ACCESS_CHANGE_USER_INFO_IN*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_REQUEST_DESKTOP:
    {
        ST_ACCESS_REQUEST_DESKTOP* param = (ST_ACCESS_REQUEST_DESKTOP*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_POWEROFF_TERMINAL:
    case TYPE_RESTART_TERMINAL:
    case TYPE_MSG_TERMINAL:
    {
        ST_ACCESS_REQUEST_DESKTOP* param = (ST_ACCESS_REQUEST_DESKTOP*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_START_DESKTOPPOOL:
    case TYPE_RESTART_DESKTOPPOOL:
    case TYPE_STOP_DESKTOPPOOL:
    case TYPE_POWERON_DESKTOP:
    case TYPE_REBOOT_DESKTOP:
    case TYPE_POWEROFF_DESKTOP:
    {
        ST_ACCESS_DESK_CONTROL_JOBID_IN* param = (ST_ACCESS_DESK_CONTROL_JOBID_IN*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TPPE_CHECK_DESKTOPPOOL_STATE:
    case TYPE_CHECK_REMOTEDESKTOP_STATE:
    {
        ST_ACCESS_CHECK_DESK_STATE* param = (ST_ACCESS_CHECK_DESK_STATE*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_ATTACH_DISK:
    {
        ST_ACCESS_ATTACH_VDISK* param = (ST_ACCESS_ATTACH_VDISK*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_QUERY_ASYN_JOB_RESULT:
    {
        ST_ACCESS_ASYN_QUERY* param = (ST_ACCESS_ASYN_QUERY*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_QUERY_CLIENT_VERSION:
    {
        ST_ACCESS_QUERY_CLIENT_VERSION* param = (ST_ACCESS_QUERY_CLIENT_VERSION*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_OPEN_CHANNEL:
    case TYPE_CLOSE_CHANNEL:
    {
        ST_CHANNEL_OP* param = (ST_CHANNEL_OP*)pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_AUTH_TOKEN:
    {
        ST_AUTH_TOKEN* param = (ST_AUTH_TOKEN*) pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    case TYPE_GET_RESPARAMTER:
    {
        GET_RES_PARAM_IN* param = (GET_RES_PARAM_IN*) pvoid;
        param->stParamCommon.pCSession = NULL;
        break;
    }
    default:
    {
        ST_ACCESS_IN_COMMON* param = (ST_ACCESS_IN_COMMON*) pvoid;
        param->pCSession = NULL;
        LOG_ERR("!!!!!!!!!!!!!!!!!!!!!MAYBE HAVEERROR!!! check carefully %d", iType);
        break;
    }

    }
    return 0;
}

int callback_csession(void* pvoid, int iType)
{
    cout<<"inParam: iType:\t"<<iType<<endl;
    switch(iType)
    {
    case TYPE_GETDOMAIN:
    {
        ST_ACCESS_GETDOMAIN_IN* param = (ST_ACCESS_GETDOMAIN_IN*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;

        if(NULL != pSession)
        {
            pSession->setSessionInfo(&(param->stParamCommon.network), NULL);
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            cout<<"callback_csession:TYPE_GETDOMAIN iUserRequestStop"<<iUserRequestStop<<endl;
            int iDomainCount = param->stDomainData.vstrDomainlists.size();
            cout<<"\tdomain count:"<<iDomainCount<<endl;
            for(int i =0; i < iDomainCount; i++)
            {
                cout<<"\tdomain("<<i<<")\t"<<param->stDomainData.vstrDomainlists[i]<<endl;
            }
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                DOMAIN_DATA* pDomainData = new(DOMAIN_DATA);
                pDomainData->vstrDomainlists = param->stDomainData.vstrDomainlists;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pDomainData);
            }            
        }
        delete param;
        param = NULL;

        break;
    }
    case TYPE_GET_USERNAME:
    {
        ST_ACCESS_GET_USERNAME *param = (ST_ACCESS_GET_USERNAME *)pvoid;
        int iUserRequestStop = 0;
        CSession *pSession = param->stParamCommon.pCSession;
        if(NULL != pSession){
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if( NULL != param->stParamCommon.callback_ui){
                if(NULL != param->stParamCommon.callbackParam){
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                }
                USERNAME_DATA *pUserNameData = new (USERNAME_DATA);
                strcpy(pUserNameData->userName , param->userName);
                if(iUserRequestStop == 0){
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void *)(pUserNameData));
                }
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_LOGIN:
    {
        ST_ACCESS_LOGIN_IN* param = (ST_ACCESS_LOGIN_IN*) pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
//            if(param->stParamCommon.iErrCode < 0)
//                pSession->setLoginStatusFailed();
            pSession->setSessionInfo(&(param->stLoginData.stNetwork), NULL);
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            cout<<"callback_csession:TYPE_LOGIN"<<endl;
            cout<<"\t logonTicket:\t"<<param->stLoginData.stLoginInfo.logonTicket<<endl;
            cout<<"\tntDomain:\t"<<param->stLoginData.stLoginInfo.ntDomain<<endl;
            cout<<"\tntUsername:\t"<<param->stLoginData.stLoginInfo.ntUsername<<endl;
            cout<<"\t:ntPassword\t"<<param->stLoginData.stLoginInfo.ntPassword<<endl;
            param->stParamCommon.pCSession->setAccountInfo(param->stLoginData.stLoginInfo);
            param->stParamCommon.pCSession->setLoginType(LOGIN_TYPE_USERNAME_PASSWD);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                LOGIN_DATA* pLoginData = new(LOGIN_DATA);
                pLoginData->stLoginInfo = param->stLoginData.stLoginInfo;
                if( iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pLoginData);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_KEEPSESSION:
    {
        cout<<"callback_csession:TYPE_KEEPSESSION"<<endl;
        ST_ACCESS_KEEPSESSION_IN* param = (ST_ACCESS_KEEPSESSION_IN*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            cout<<"callback_csession:timeout:"<<param->st_keepSession.i_timeOut<<endl;
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                KEEP_SESSION_DATA* pKeepSession = new(KEEP_SESSION_DATA);
                pKeepSession->i_timeOut = param->st_keepSession.i_timeOut;
                if( iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pKeepSession);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_LOGOUT:
    {
        cout<<"callback_csession:TYPE_LOGOUT"<<endl;
        ST_ACCESS_LOGOUT_IN* param = (ST_ACCESS_LOGOUT_IN*) pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                pSession->setLoginType(LOGIN_TYPE_UNKNOWN);
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                LOGOUT_DATA* pLogoutData = new (LOGOUT_DATA);
                pLogoutData->iErrorCode = param->stParamCommon.iErrCode;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, pLogoutData);
            }
        }
        delete param;
        param = NULL;
        break;
    }
#ifndef _WIN32
    case TYPE_SWITCH_ACCESS:
    {
        cout<<"callback_csession:TYPE_SWITCH_ACCESS"<<endl;
        ST_ACCESS_LOGOUT_IN* param = (ST_ACCESS_LOGOUT_IN*) pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                pSession->setLoginType(LOGIN_TYPE_UNKNOWN);
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                LOGOUT_DATA* pLogoutData = new (LOGOUT_DATA);
                pLogoutData->iErrorCode = param->stParamCommon.iErrCode;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, pLogoutData);
            }
        }
        delete param;
        param = NULL;
        break;
    }
#endif
    case TYPE_GETUSERINFO:
    {
        cout<<"callback_csession:TYPE_GETUSERINFO"<<endl;
        ST_ACCESS_GETUSERINFO_IN* param = (ST_ACCESS_GETUSERINFO_IN*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {

            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            int diskCount = param->st_GetuserInfo.vstVirtualDisks.size();
            cout<<"callback_csession:diskCount:"<<diskCount<<endl;
            for(int i = 0; i < diskCount; i++)
            {
                cout<<"\t disk("<<i<<"):"<<endl;
                VIRTUALDISK disk = param->st_GetuserInfo.vstVirtualDisks[i];
                cout<<"\t\t\t\t:devicePath:\t"<< disk.devicePath<<endl;
                cout<<"\t\t\t\t:diskSize:\t"<< disk.diskSize <<endl;
                cout<<"\t\t\t\t:sizeUnit:\t"<< disk.sizeUnit <<endl;
            }
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                GET_USER_INFO_DATA* pGetUserInfo = new(GET_USER_INFO_DATA);
                pGetUserInfo->vstVirtualDisks = param->st_GetuserInfo.vstVirtualDisks;
                pGetUserInfo->stNtAccountInfo = param->st_GetuserInfo.stNtAccountInfo;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pGetUserInfo);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_GETICON:
    {
        ST_ACCESS_GETICON_IN* param = (ST_ACCESS_GETICON_IN*)pvoid;
        LOG_INFO("TYPE_GETICON:\nicon len:\t%d", param->stGetIcon.iLen);
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {

            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                GET_ICON_DATA* pIconData = new(GET_ICON_DATA);
                pIconData->data = param->stGetIcon.data;
                pIconData->iLen = param->stGetIcon.iLen;
                pIconData->strAppUuid = param->stGetIcon.strAppUuid;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pIconData);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_LIST_USER_RES:
    {
        ST_ACCESS_LISTRES_IN* param = (ST_ACCESS_LISTRES_IN*)pvoid;
//        LOG_INFO("res number:\t%d", param->sListUserRes.stAppList.size());
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                LIST_USER_RESOURCE_DATA* pRes = new(LIST_USER_RESOURCE_DATA);
                pRes->stAppList = param->sListUserRes.stAppList;
                pRes->stAppBakList = param->sListUserRes.stAppBakList; // for terminal init;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pRes);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_LAUNCH_RES:
    {
        ST_ACCESS_LAUNCHRES_IN* param = (ST_ACCESS_LAUNCHRES_IN*)pvoid;
        LOG_INFO("resource IP:%s, port:%s, resTicket:%s", param->stLaunchResData.stResInfo.ipAddr,\
                 param->stLaunchResData.stResInfo.port, param->stLaunchResData.stResInfo.resourceTicket);
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                LAUNCH_RESOURCE_DATA* pLaunchRes = new(LAUNCH_RESOURCE_DATA);
                pLaunchRes->stResInfo = param->stLaunchResData.stResInfo;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pLaunchRes);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_SHUTDOWN_RES:
    {
        ST_ACCESS_SHUTDOWN_RES_IN* param =(ST_ACCESS_SHUTDOWN_RES_IN*)pvoid;
        LOG_INFO("shutdown resource: resTicket:%s return value:%d", param->str_resTicket.c_str(), param->stParamCommon.iErrCode);
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, NULL);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_CHANGE_USER_INFO:
    {
        ST_ACCESS_CHANGE_USER_INFO_IN* param =(ST_ACCESS_CHANGE_USER_INFO_IN*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
//            param->stParamCommon.pCSession->setAccountInfo(param->stAccountInfo);
            param->stParamCommon.pCSession->setUserInfo(param->stUserInfo);
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, NULL);
            }
        }
        delete param;
        param = NULL;
        break;

    }
    case TYPE_CHANGE_NTUSER_INFO:
    {
        ST_ACCESS_CHANGE_USER_INFO_IN* param =(ST_ACCESS_CHANGE_USER_INFO_IN*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
            param->stParamCommon.pCSession->setAccountInfo(param->stAccountInfo);
            param->stParamCommon.pCSession->setUserInfo(param->stUserInfo);
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, NULL);
            }
        }
        delete param;
        param = NULL;
        break;

    }
    case TYPE_POWEROFF_TERMINAL:
    case TYPE_RESTART_TERMINAL:
    case TYPE_MSG_TERMINAL:
    {
        ST_ACCESS_TERMINAL_DESKTOP_CTL* param = (ST_ACCESS_TERMINAL_DESKTOP_CTL*)pvoid;
               int iTerminalCtl = 0;
               CSession* pSession = param->stParamCommon.pCSession;
               if( NULL != pSession )
               {
                   pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid,&iTerminalCtl);
                   if( NULL != param->stParamCommon.callback_ui)
                   {
                       if( NULL != param->stParamCommon.callbackParam)
                       {
                           param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;

                       }
                       if(iTerminalCtl == 0)
                       {
                           (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam,param->stParamCommon.type,NULL);
                       }
                   }
               }
               delete param;
               param = NULL;
               break;
    }
    case TYPE_REQUEST_DESKTOP:
    {
        ST_ACCESS_REQUEST_DESKTOP* param = (ST_ACCESS_REQUEST_DESKTOP*)pvoid;
        int iRequestDesktop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if( NULL != pSession )
        {
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid,&iRequestDesktop);
            if( NULL != param->stParamCommon.callback_ui)
            {
                if( NULL != param->stParamCommon.callbackParam)
                {
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;

                }
                if(iRequestDesktop == 0)
                {
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam,param->stParamCommon.type,NULL);
                }
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_START_DESKTOPPOOL:
    case TYPE_RESTART_DESKTOPPOOL:    
    case TYPE_STOP_DESKTOPPOOL:
    {
        ST_ACCESS_DESK_CONTROL_JOBID_IN* param = (ST_ACCESS_DESK_CONTROL_JOBID_IN*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        CHECK_DESK_STATE_DATA stCheckResult;
        stCheckResult.strJobId = param->strUuid;
        stCheckResult.iRdpState = 0;
        stCheckResult.iState = 0;
        if(NULL != pSession)
        {
            CALLBACK_PARAM_UI stCall_param;
            stCall_param.errorCode = 0;
            PARAM_SESSION_IN param_Query;
            param_Query.callback_param = &stCall_param;
            param_Query.isBlock = BLOCKED;
            ASYN_QUERY_DATA st_queryResult;
            int count = 0;
            if(0 != param->stParamCommon.iErrCode)
            {
                LOG_ERR("FAILED errocode:%d\t desktoptype:", param->stParamCommon.iErrCode, iType);
            }
            while(0 == param->stParamCommon.iErrCode)//(re)start or stop execute succeed then goto asyn query
            {//asyn query...
                if(count >= 200)
                {
                    param->stParamCommon.iErrCode = ERROR_DESKTOPPOOL_UNREACHABLE;
                    break;
                }
                if(NULL == param->stParamCommon.pCSession)
                    break;
                pSession->queryAsynJobResult(param_Query, param->stDeskControlJobId.strJobId.c_str(), &st_queryResult);
                if(0 != stCall_param.errorCode)
                {
                    param->stParamCommon.iErrCode = stCall_param.errorCode;
                    LOG_ERR("queryAsynJobResult failed, return value:", stCall_param.errorCode);
                    break;
                }
                if(-1==st_queryResult.iJobStatus || 2 == st_queryResult.iJobStatus)
                {
                    if(NULL == param->stParamCommon.pCSession)
                        break;
                    pSession->checkDesktopPoolState(param_Query, param->strUuid.c_str(), &stCheckResult);
                    param->stParamCommon.iErrCode = ERROR_FAIL;
                    break;
                }
                else if(1 == st_queryResult.iJobStatus)
                {//query desktop state
                    int iQueryTimes = 0;
                    while(true)
                    {
                        if(NULL == param->stParamCommon.pCSession)
                            break;
                        pSession->checkDesktopPoolState(param_Query, param->strUuid.c_str(), &stCheckResult);
                        if(0 != stCall_param.errorCode)
                        {
                            param->stParamCommon.iErrCode = stCall_param.errorCode;
                            LOG_ERR("queryAsynJobResult failed, return value:", stCall_param.errorCode);
                            break;
                        }
                        if(TYPE_STOP_DESKTOPPOOL == iType)
                        {
                            if(1 != stCheckResult.iState)//-1 unreachable 0 has stopped 1 running 2 rdp has openend
                                break;
                        }
                        else
                        {
                            if(0 != stCheckResult.iState)
                                break;
                        }
                        Sleep(5000);
                        iQueryTimes++;
                    }
                    break;
                }

                Sleep(3000);
                count++;
            }
            if(NULL != param->stParamCommon.pCSession)
            {
                pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
                if(NULL != param->stParamCommon.callback_ui)
                {
                    if(NULL != param->stParamCommon.callbackParam)
                        param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                    CHECK_DESK_STATE_DATA* pstCheckResult = new(CHECK_DESK_STATE_DATA);
                    *pstCheckResult = stCheckResult;
                    (*pstCheckResult).strJobId = param->strUuid;
                    if(iUserRequestStop == 0 && NULL != param->stParamCommon.pCSession)
                        (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pstCheckResult);
                }
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_POWERON_DESKTOP:
    case TYPE_REBOOT_DESKTOP:
    case TYPE_POWEROFF_DESKTOP:
    {
        ST_ACCESS_DESK_CONTROL_JOBID_IN* param = (ST_ACCESS_DESK_CONTROL_JOBID_IN*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        CHECK_DESK_STATE_DATA stCheckResult;
        stCheckResult.strJobId = param->strUuid;
        stCheckResult.iRdpState = 0;
        stCheckResult.iState = 0;
        if(NULL != param->stParamCommon.pCSession)
        {
            CALLBACK_PARAM_UI stCall_param;
            stCall_param.errorCode = 0;
            PARAM_SESSION_IN param_Query;
            param_Query.callback_param = &stCall_param;
            param_Query.isBlock = BLOCKED;
            ASYN_QUERY_DATA st_queryResult;
            int count = 0;
            if(0 != param->stParamCommon.iErrCode)
            {
                LOG_ERR("FAILED errocode:%d\t desktoptype:", param->stParamCommon.iErrCode, iType);
            }
            while(0 == param->stParamCommon.iErrCode)//(re)start or stop execute succeed then goto asyn query
            {//asyn query...
                if(count >= 200)
                {
                    param->stParamCommon.iErrCode = ERROR_DESKTOPPOOL_UNREACHABLE;
                    break;
                }
                if(NULL == param->stParamCommon.pCSession)
                    break;
                pSession->queryAsynJobResult(param_Query, param->stDeskControlJobId.strJobId.c_str(), &st_queryResult);
                if(0 != stCall_param.errorCode)
                {
                    param->stParamCommon.iErrCode = stCall_param.errorCode;
                    LOG_ERR("queryAsynJobResult failed, return value:", stCall_param.errorCode);
                    break;
                }
                if(-1==st_queryResult.iJobStatus || 2 == st_queryResult.iJobStatus)
                {
                    if(NULL == param->stParamCommon.pCSession)
                        break;
                    pSession->checkRemoteDesktopState(param_Query, param->strUuid.c_str(), &stCheckResult);   //if selfservice fail,it will show
                    param->stParamCommon.iErrCode = ERROR_FAIL;
                    break;
                }
                else if(1 == st_queryResult.iJobStatus)
                {//query desktop state
                    int iQueryTimes = 0;
                    while(true)
                    {
                        if(NULL == param->stParamCommon.pCSession)
                            break;
                        pSession->checkRemoteDesktopState(param_Query, param->strUuid.c_str(), &stCheckResult);
                        if(0 != stCall_param.errorCode)
                        {
                            param->stParamCommon.iErrCode = stCall_param.errorCode;
                            LOG_ERR("queryAsynJobResult failed, return value:", stCall_param.errorCode);
                            break;
                        }
                        if(TYPE_POWEROFF_DESKTOP == iType)
                        {
                            if(1 != stCheckResult.iState)//-1 unreachable 0 has stopped 1 running 2 rdp has openend
                                break;
                        }
                        else
                        {
                            if(0 != stCheckResult.iState)
                                break;
                        }
                        Sleep(5000);
                        iQueryTimes++;
                    }
                    break;
                }

                Sleep(3000);
                count++;
            }
            if(NULL != param->stParamCommon.pCSession)
            {
                pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
                if(NULL != param->stParamCommon.callback_ui)
                {
                    if(NULL != param->stParamCommon.callbackParam)
                        param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                    CHECK_DESK_STATE_DATA* pstCheckResult = new(CHECK_DESK_STATE_DATA);
                    *pstCheckResult = stCheckResult;
                    (*pstCheckResult).strJobId = param->strUuid;
                    if(iUserRequestStop == 0 && NULL != param->stParamCommon.pCSession)
                        (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pstCheckResult);
                }
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TPPE_CHECK_DESKTOPPOOL_STATE:
    case TYPE_CHECK_REMOTEDESKTOP_STATE:
    {
        ST_ACCESS_CHECK_DESK_STATE* param = (ST_ACCESS_CHECK_DESK_STATE*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                CHECK_DESK_STATE_DATA* pStateData = new(CHECK_DESK_STATE_DATA);
    //            pStateData->iRdpState = param->stCheckDeskState.iRdpState;
    //            pStateData->iState = param->stCheckDeskState.iState;
    //            pStateData->strIp = param->stCheckDeskState.strIp;
                *pStateData = param->stCheckDeskState;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pStateData);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_ATTACH_DISK:
    {
        ST_ACCESS_ATTACH_VDISK* param = (ST_ACCESS_ATTACH_VDISK*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                ATTACH_VDISK_DATA* pVDiskData = new(ATTACH_VDISK_DATA);
                *pVDiskData = param->stAttachVDisk;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pVDiskData);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_QUERY_ASYN_JOB_RESULT:
    {
        ST_ACCESS_ASYN_QUERY* param = (ST_ACCESS_ASYN_QUERY*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {

            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                ASYN_QUERY_DATA* pAsynQueryData = new(ASYN_QUERY_DATA);
                *pAsynQueryData = param->stAsynQuery;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pAsynQueryData);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_QUERY_CLIENT_VERSION:
    {
        ST_ACCESS_QUERY_CLIENT_VERSION* param = (ST_ACCESS_QUERY_CLIENT_VERSION*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {

            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                QUERY_VERSION_DATA* pVersionData = new(QUERY_VERSION_DATA);
                *pVersionData = param->stQueryVersion;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pVersionData);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_OPEN_CHANNEL:
    case TYPE_CLOSE_CHANNEL:
    {
        ST_CHANNEL_OP* param = (ST_CHANNEL_OP*)pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {

            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                CHANNEL_OP_DATA* pChannelOpData = new(CHANNEL_OP_DATA);;
                *pChannelOpData = param->st_channel_op;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pChannelOpData);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_AUTH_TOKEN:
    {
        ST_AUTH_TOKEN* param = (ST_AUTH_TOKEN*) pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
//            if(param->stParamCommon.iErrCode < 0)
//                pSession->setLoginStatusFailed();
            pSession->setSessionInfo(&(param->stLoginData.stNetwork), NULL);
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            cout<<"callback_csession:TYPE_LOGIN"<<endl;
            cout<<"\t logonTicket:\t"<<param->stLoginData.stLoginInfo.logonTicket<<endl;
            cout<<"\tntDomain:\t"<<param->stLoginData.stLoginInfo.ntDomain<<endl;
            cout<<"\tntUsername:\t"<<param->stLoginData.stLoginInfo.ntUsername<<endl;
            cout<<"\t:ntPassword\t"<<param->stLoginData.stLoginInfo.ntPassword<<endl;
            param->stParamCommon.pCSession->setAccountInfo(param->stLoginData.stLoginInfo);
            param->stParamCommon.pCSession->setLoginType(LOGIN_TYPE_TOKEN_AUTH);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                LOGIN_DATA_TOKEN* pAuthTokenData = new(LOGIN_DATA_TOKEN);
                pAuthTokenData->stLoginInfo = param->stLoginData.stLoginInfo;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pAuthTokenData);
            }
        }
        delete param;
        param = NULL;
        break;
    }
    case TYPE_GET_RESPARAMTER:
    {
        GET_RES_PARAM_IN* param = (GET_RES_PARAM_IN*) pvoid;
        int iUserRequestStop = 0;
        CSession* pSession = param->stParamCommon.pCSession;
        if(NULL != pSession)
        {
            pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
            if(NULL != param->stParamCommon.callback_ui)
            {
                if(NULL != param->stParamCommon.callbackParam)
                    param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
                GET_RESOURCE_PARAMETER* pGetResParamOut = new(GET_RESOURCE_PARAMETER);
                *pGetResParamOut = param->stResParm_out;
                if(iUserRequestStop == 0)
                    (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pGetResParamOut);
            }
        }
        delete param;
        param = NULL;
        break;
    }

    }

    return 0;
}

int CSession::setSessionInfo(const NETWORK* const pNetwork, const USER_INFO* const pUserinfo)
{
//    if( 0 != m_iHasLogin)
//    {//only allow to set m_stNetwork m_stUserInfo before login
//        return -1;
//    }
    if(NULL != pNetwork)
    {
        m_stNetwork = *pNetwork;
    }
    if( NULL != pUserinfo)
    {
        m_stUserInfo = *pUserinfo;
    }
    return 0;
}


//*****************************************************************
//function Name:getDomainList
//parameter:
//	st_param_in(PARAM_SESSION_IN) [in]:
//      specifies the request type, call back function ...(more details
//      please reference to definition of PARAM_SESSION_IN)
//	pDomainData(DomainData*)[out]:
//      if isBlock( located in PARAM_SESSION_IN) is BLOCKED,
//      this function will return until get the result, the result is put in pDomainData,
//      the caller is responsible to release pDomainData(call delete(pDomainData))
//return value:
//	>= 0		succeed
//	< 0			failed
//*****************************************************************
int CSession::getDomainList(PARAM_SESSION_IN &st_param_in, DOMAIN_DATA *pDomainData)
{
    ST_ACCESS_GETDOMAIN_IN* param = new(ST_ACCESS_GETDOMAIN_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_GETDOMAIN;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessGetDomainList(param);
        cout<<"count:"<<param->stDomainData.vstrDomainlists.size()<<endl;
        if(NULL == pDomainData)
        {
            pDomainData = new(DOMAIN_DATA);
        }
        pDomainData->vstrDomainlists=param->stDomainData.vstrDomainlists;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessGetDomainList, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}
int CSession::getUserName(PARAM_SESSION_IN &st_param_in, char uuid[512],USERNAME_DATA *pUserName)
{
    ST_ACCESS_GET_USERNAME *param = new(ST_ACCESS_GET_USERNAME);
    memset(param,0, sizeof(ST_ACCESS_GET_USERNAME));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_GET_USERNAME;
    param->stParamCommon.pCSession = this;
    strcpy(param->uuid, uuid);
    st_param_in.taskUuid = TASK_UUID_NULL;

    if(param->stParamCommon.bIsBlocked == BLOCKED){
        VAccessGetUserName(param);
        if(pUserName == NULL){
            pUserName = new(USERNAME_DATA);
        }
        strcpy(pUserName->userName, param->userName);
        if(NULL != st_param_in.callback_param){
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete param;
    }else{
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessGetUserName, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}
int CSession::loginSession(PARAM_SESSION_IN& st_param_in, LOGIN_DATA* pLoginData/* = NULL*/, bool bAutoLogin)
{
    //setLoginStatusSucceed();
    ST_ACCESS_LOGIN_IN* param = new(ST_ACCESS_LOGIN_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stUserInfo = m_stUserInfo;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_LOGIN;
    param->stParamCommon.pCSession = this;
    param->bAutoLogin = bAutoLogin;
    st_param_in.taskUuid = TASK_UUID_NULL;

    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessLoginSession(param);
        if(NULL == pLoginData)
        {
            pLoginData = new(LOGIN_DATA) ;
        }
        pLoginData->stLoginInfo=param->stLoginData.stLoginInfo; // nt account info
        setAccountInfo(param->stLoginData.stLoginInfo);
        cout<<"session Ticket:\t"<<getSessionTicket()<<endl;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete param;
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessLoginSession, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;

}

int CSession::logoutSession(PARAM_SESSION_IN& st_param_in)
{
    ST_ACCESS_LOGOUT_IN* param = new(ST_ACCESS_LOGOUT_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_LOGOUT;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    param->str_SessionTicket = m_accountInfo.logonTicket;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessLogoutSession(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessLogoutSession, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::switchAccessSession(PARAM_SESSION_IN& st_param_in)
{
    cerr << "Yeah! we got here. CSession::" << endl;

    ST_ACCESS_LOGOUT_IN* param = new(ST_ACCESS_LOGOUT_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_SWITCH_ACCESS;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    param->str_SessionTicket = m_accountInfo.logonTicket;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessSwitchAccessSession(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        cerr << "make sure we got here" << endl;
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessSwitchAccessSession, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }

    // DO WE NEED TO CONNECT TO THE NEW VACCESS HERE?

    return 0;
}

int CSession::getUserInfo(PARAM_SESSION_IN& st_param_in, GET_USER_INFO_DATA* pGetUserInfo/* = NULL*/)
{
    ST_ACCESS_GETUSERINFO_IN* param = new(ST_ACCESS_GETUSERINFO_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_GETUSERINFO;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessGetUserInfo(param);
        cout<<"disknum:"<<param->st_GetuserInfo.vstVirtualDisks.size()<<endl;
        if(NULL == pGetUserInfo)
        {
            pGetUserInfo = new (GET_USER_INFO_DATA);
        }
        pGetUserInfo->vstVirtualDisks=param->st_GetuserInfo.vstVirtualDisks;
        pGetUserInfo->stNtAccountInfo=param->st_GetuserInfo.stNtAccountInfo;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessGetUserInfo, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::getIcon(PARAM_SESSION_IN& st_param_in, char* appUuid, GET_ICON_DATA* pIconData/* = NULL*/)
{
    if(NULL == appUuid)
    {
        LOG_ERR("%s","NULL == appUuid");
        return -1;
    }
    ST_ACCESS_GETICON_IN* param = new(ST_ACCESS_GETICON_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_GETICON;
    param->strUrlPath = param->strUrlPath + "/appicon/" + appUuid + ".png";
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    param->stGetIcon.strAppUuid = appUuid;
    param->stGetIcon.data = NULL;
    param->stGetIcon.iLen = 0;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessGetIcon(param);
        LOG_INFO("icon len:\t%d", param->stGetIcon.iLen);
        if(NULL == pIconData)
        {
            pIconData = new (GET_ICON_DATA);
        }
        pIconData->data = param->stGetIcon.data;
        pIconData->iLen = param->stGetIcon.iLen;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessGetIcon, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::listUserResource(PARAM_SESSION_IN& st_param_in, bool bGetResParam, LIST_USER_RESOURCE_DATA* pRes/* = NULL*/)
{
    ST_ACCESS_LISTRES_IN* param = new(ST_ACCESS_LISTRES_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->b_getResParam = bGetResParam;
    param->stParamCommon.type = TYPE_LIST_USER_RES;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessListUserResource(param);
        LOG_INFO("res number:\t%d", param->sListUserRes.stAppList.size());
        if(NULL == pRes)
        {
            pRes = new (LIST_USER_RESOURCE_DATA);
        }
        pRes->stAppList = param->sListUserRes.stAppList;
        pRes->stAppBakList = param->sListUserRes.stAppBakList; // for terminal refresh
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessListUserResource, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::launchResource(PARAM_SESSION_IN& st_param_in, PARAM_LAUNCH_RES_IN& st_launchRes, LAUNCH_RESOURCE_DATA* pLaunchRes/* = NULL*/)
{
    ST_ACCESS_LAUNCHRES_IN* param = new(ST_ACCESS_LAUNCHRES_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession; // callback_csession()
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_LAUNCH_RES;
    param->stParamCommon.pCSession = this;
    param->str_SessionTicket = getSessionTicket();

    param->strResUuid = st_launchRes.strResUuid;
    param->iResType = st_launchRes.iResType;
    param->iDisplayProtocol = st_launchRes.iDisplayProtocol;
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessLaunchResource(param);
       // cout<<"launch res errocode:"<<param->stParamCommon.iErrCode<<endl;
        LOG_INFO("resource IP:%s, port:%s, resTicket:%s", param->stLaunchResData.stResInfo.ipAddr, param->stLaunchResData.stResInfo.port, param->stLaunchResData.stResInfo.resourceTicket);
        if(NULL == pLaunchRes)
        {
            pLaunchRes= new (LAUNCH_RESOURCE_DATA);
        }
        pLaunchRes->stResInfo = param->stLaunchResData.stResInfo;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessLaunchResource, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::shutdownRes(PARAM_SESSION_IN& st_param_in, char *pResTicket, int iIsRelease)
{
    if(NULL == pResTicket)
    {
        LOG_ERR("%s","NULL == pResTicket");
        return -1;
    }
    AccessShutdownRes* param = new(AccessShutdownRes);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_SHUTDOWN_RES;
    param->str_SessionTicket = getSessionTicket();
    param->str_resTicket = pResTicket;
    param->iIsRelease = iIsRelease;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessShutdownResource(param);
        LOG_INFO("shutdown resource: resTicket:%s return value:%d", param->str_resTicket.c_str(), param->stParamCommon.iErrCode);
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessShutdownResource, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}



int CSession::changeUserInfo(PARAM_SESSION_IN& st_param_in, PARAM_CHANGE_USER_INFO_IN& st_changeUserInfo)
{
    ST_ACCESS_CHANGE_USER_INFO_IN* param = new(ST_ACCESS_CHANGE_USER_INFO_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_CHANGE_USER_INFO;
    param->stParamCommon.pCSession = this;

    param->stAccountInfo = getNT_ACCOUNT_INFO();
    param->stUserInfo = getUSER_INFO();
//    param->strNtPasswd = st_changeUserInfo.strNtPasswd;
//    param->strNtUserName = st_changeUserInfo.strNtUserName;
    param->strPasswd = st_changeUserInfo.strPasswd;
    param->strNewPasswd = st_changeUserInfo.strNewPasswd;

    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessChangeUserInfo(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessChangeUserInfo, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::changeNtUserInfo(PARAM_SESSION_IN& st_param_in, PARAM_CHANGE_USER_INFO_IN& st_changeUserInfo)
{
    ST_ACCESS_CHANGE_USER_INFO_IN* param = new(ST_ACCESS_CHANGE_USER_INFO_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_CHANGE_NTUSER_INFO;
    param->stParamCommon.pCSession = this;

    param->stAccountInfo = getNT_ACCOUNT_INFO();
    param->stUserInfo = getUSER_INFO();
    param->strNtPasswd = st_changeUserInfo.strNtPasswd;
    param->strNtUserName = st_changeUserInfo.strNtUserName;
    param->strPasswd = st_changeUserInfo.strPasswd;
    param->strNewPasswd = st_changeUserInfo.strNewPasswd;

    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessChangeNtUserInfo(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessChangeNtUserInfo, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::requestDesktop(PARAM_SESSION_IN &st_param_in, REQUEST_DESKTOP &requestDesktop)
{
    ST_ACCESS_REQUEST_DESKTOP* param = new ST_ACCESS_REQUEST_DESKTOP;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_REQUEST_DESKTOP;
    param->stParamCommon.pCSession = this;

    param->Cpu = requestDesktop.Cpu;
    param->Disk = requestDesktop.Disk;
    param->desktopDescrip = requestDesktop.desktopDescrip;
    param->Memory = requestDesktop.Memory;
    strcpy(param->logonTicket, requestDesktop.logonTicket);
    param->Os = requestDesktop.Os;

    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessRequestDesktop(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type,(FUN_THREAD)VAccessRequestDesktop,param,&(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::powerOffTerminal(PARAM_SESSION_IN &st_param_in, TERMINAL_DESKTOP &terminalDesktop)
{
    ST_ACCESS_TERMINAL_DESKTOP_CTL* param = new ST_ACCESS_TERMINAL_DESKTOP_CTL;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_POWEROFF_TERMINAL;
    param->stParamCommon.pCSession = this;
    strcpy(param->loginTicket, terminalDesktop.loginTicket);
    for(unsigned int loop = 0; loop < terminalDesktop.uuid.size(); loop++)
    {
        param->uuid.push_back(terminalDesktop.uuid[loop]);
    }
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessPoweroffTerminalDesktop(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type,(FUN_THREAD)VAccessPoweroffTerminalDesktop,param,&(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::restartTerminal(PARAM_SESSION_IN &st_param_in, TERMINAL_DESKTOP &terminalDesktop)
{
    ST_ACCESS_TERMINAL_DESKTOP_CTL* param = new ST_ACCESS_TERMINAL_DESKTOP_CTL;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_RESTART_TERMINAL;
    param->stParamCommon.pCSession = this;

    strcpy(param->loginTicket, terminalDesktop.loginTicket);
    for(unsigned int loop = 0; loop < terminalDesktop.uuid.size(); loop++)
    {
        param->uuid.push_back(terminalDesktop.uuid[loop]);
    }
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessRestartTerminalDesktop(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type,(FUN_THREAD)VAccessRestartTerminalDesktop,param,&(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}
int CSession::messageTerminal(PARAM_SESSION_IN &st_param_in, TERMINAL_DESKTOP &terminalDesktop)
{
    ST_ACCESS_TERMINAL_DESKTOP_CTL* param = new ST_ACCESS_TERMINAL_DESKTOP_CTL;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_MSG_TERMINAL;
    param->stParamCommon.pCSession = this;

    strcpy(param->loginTicket, terminalDesktop.loginTicket);
    param->msg = terminalDesktop.msg;
    for(unsigned int loop = 0; loop < terminalDesktop.uuid.size(); loop++)
    {
//        strcpy(param->uuid[loop], terminalDesktop.uuid[loop]);
        param->uuid.push_back(terminalDesktop.uuid[loop]);
    }
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessMsgTerminalDesktop(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type,(FUN_THREAD)VAccessMsgTerminalDesktop,param,&(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::startDesktopPool(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData/* = NULL*/)
{
    if(NULL == pchUuid)
    {
        LOG_ERR("%s","NULL == pchUuid");
        return -1;
    }
    ST_ACCESS_DESK_CONTROL_JOBID_IN* param = new(ST_ACCESS_DESK_CONTROL_JOBID_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_START_DESKTOPPOOL;
    param->stParamCommon.pCSession = this;

    param->strUuid = pchUuid;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessStartDesktopPool(param);
        if(NULL == pControlJobData)
        {
            pControlJobData = new(DESK_CONTROL_JOBID_DATA);
        }
        pControlJobData->strJobId = param->stDeskControlJobId.strJobId;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessStartDesktopPool, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;

}

int CSession::restartDesktopPool(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData/* = NULL*/)
{
    if(NULL == pchUuid)
    {
        LOG_ERR("%s","NULL == pchUuid");
        return -1;
    }
    ST_ACCESS_DESK_CONTROL_JOBID_IN* param = new(ST_ACCESS_DESK_CONTROL_JOBID_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_RESTART_DESKTOPPOOL;
    param->stParamCommon.pCSession = this;

    param->strUuid = pchUuid;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessRestartDesktopPool(param);
        if(NULL == pControlJobData)
        {
            pControlJobData = new(DESK_CONTROL_JOBID_DATA);
        }
        pControlJobData->strJobId = param->stDeskControlJobId.strJobId;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessRestartDesktopPool, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::stopDesktopPool(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData/* = NULL*/)
{
    if(NULL == pchUuid)
    {
        LOG_ERR("%s","NULL == pchUuid");
        return -1;
    }
    ST_ACCESS_DESK_CONTROL_JOBID_IN* param = new(ST_ACCESS_DESK_CONTROL_JOBID_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_STOP_DESKTOPPOOL;
    param->stParamCommon.pCSession = this;

    param->strUuid = pchUuid;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessStopDesktopPool(param);
        if(NULL == pControlJobData)
        {
            pControlJobData = new(DESK_CONTROL_JOBID_DATA);
        }
        pControlJobData->strJobId = param->stDeskControlJobId.strJobId;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessStopDesktopPool, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::checkDesktopPoolState(PARAM_SESSION_IN& st_param_in, const char* pchUuid, CHECK_DESK_STATE_DATA* pCheckDeskStateData/* = NULL*/)
{
    if(NULL == pchUuid)
    {
        LOG_ERR("%s","NULL == pchUuid");
        return -1;
    }
    ST_ACCESS_CHECK_DESK_STATE* param = new(ST_ACCESS_CHECK_DESK_STATE);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TPPE_CHECK_DESKTOPPOOL_STATE;
    param->stParamCommon.pCSession = this;

    param->str_deskUuid = pchUuid;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stCheckDeskState.iRdpState = 0;
    param->stCheckDeskState.iState = 0;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessCheckDesktopPoolState(param);
        if(NULL == pCheckDeskStateData)
        {
            pCheckDeskStateData = new(CHECK_DESK_STATE_DATA);
        }
        *pCheckDeskStateData = param->stCheckDeskState;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessCheckDesktopPoolState, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::attachVDisk(PARAM_SESSION_IN& st_param_in, const char *pchUuid, int desktopType, const VIRTUALDISK& vDisk,  ATTACH_VDISK_DATA* pVDiskData/* = NULL*/)
{
    if(NULL == pchUuid)
    {
        LOG_ERR("%s","NULL == pchUuid");
        return -1;
    }
    ST_ACCESS_ATTACH_VDISK* param = new(ST_ACCESS_ATTACH_VDISK);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_ATTACH_DISK;
    param->stParamCommon.pCSession = this;

    //non-required: devicePath
//    memset(vDisk.devicePath, 0, sizeof(vDisk.devicePath));
    param->devicePath = vDisk.devicePath;
    param->str_desktopUuid = pchUuid;
    param->str_SessionTicket = getSessionTicket();
    param->iDesktopType = desktopType;
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessAttachVirtualDiskToDesktop(param);//VAccessAttachVirtualDisk(param);
        if(NULL == pVDiskData)
        {
            pVDiskData = new(ATTACH_VDISK_DATA);
        }
        *pVDiskData = param->stAttachVDisk;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessAttachVirtualDiskToDesktop, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::queryAsynJobResult(PARAM_SESSION_IN& st_param_in, const char* pchUuid, ASYN_QUERY_DATA* pAsynQueryData/* = NULL*/)
{
    if(NULL == pchUuid)
    {
        LOG_ERR("%s","NULL == pchUuid");
        return -1;
    }
    ST_ACCESS_ASYN_QUERY* param = new(ST_ACCESS_ASYN_QUERY);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_QUERY_ASYN_JOB_RESULT;
    param->stParamCommon.pCSession = this;

    param->strJobId = pchUuid;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessQueryAsyncJobResult(param);
        if(NULL == pAsynQueryData)
        {
            pAsynQueryData = new(ASYN_QUERY_DATA);
        }
        *pAsynQueryData = param->stAsynQuery;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessQueryAsyncJobResult, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::queryClientVersion(PARAM_SESSION_IN& st_param_in, int iClientType, QUERY_VERSION_DATA* pVersionData/* = NULL*/)
{
    ST_ACCESS_QUERY_CLIENT_VERSION* param = new(ST_ACCESS_QUERY_CLIENT_VERSION);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_QUERY_CLIENT_VERSION;
    param->stParamCommon.pCSession = this;

    param->iClientType = iClientType;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessQueryClientVersion(param);
        if(NULL == pVersionData)
        {
            pVersionData = new(QUERY_VERSION_DATA);
        }
        *pVersionData = param->stQueryVersion;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessQueryClientVersion, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::openChannel(PARAM_SESSION_IN& st_param_in, PARAM_CHANNELOP_IN& st_channelData, CHANNEL_OP_DATA* pChannelOpData/* = NULL*/)
{
    ST_CHANNEL_OP* param = new(ST_CHANNEL_OP);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_OPEN_CHANNEL;
    param->stParamCommon.pCSession = this;

    param->str_ip = st_channelData.str_ip;
    param->port = st_channelData.iPort;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessOpenChannel(param);
        if(NULL == pChannelOpData)
        {
            pChannelOpData = new(CHANNEL_OP_DATA);
        }
        *pChannelOpData = param->st_channel_op;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessOpenChannel, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::closeChannel(PARAM_SESSION_IN& st_param_in, PARAM_CHANNELOP_IN& st_channelData, CHANNEL_OP_DATA* pChannelOpData/* = NULL*/)
{
    ST_CHANNEL_OP* param = new(ST_CHANNEL_OP);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_CLOSE_CHANNEL;
    param->stParamCommon.pCSession = this;

    param->str_ip = st_channelData.str_ip;
    param->port = st_channelData.iPort;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessCloseChannel(param);
        if(NULL == pChannelOpData)
        {
            pChannelOpData = new(CHANNEL_OP_DATA);
        }
        *pChannelOpData = param->st_channel_op;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessCloseChannel, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::authToken(PARAM_SESSION_IN& st_param_in, LOGIN_DATA_TOKEN* pAuthTokenData/* = NULL*/)
{
    //setLoginStatusSucceed();
    ST_AUTH_TOKEN* param = new(ST_AUTH_TOKEN);
    param->stParamCommon.network = m_stNetwork;
    param->stUserInfo = m_stUserInfo;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_AUTH_TOKEN;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;

    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessAuthToken(param);
        if(NULL == pAuthTokenData)
        {
            pAuthTokenData = new(LOGIN_DATA_TOKEN) ;
        }
        pAuthTokenData->stLoginInfo=param->stLoginData.stLoginInfo;
        setAccountInfo(param->stLoginData.stLoginInfo);
        cout<<"session Ticket:\t"<<getSessionTicket()<<endl;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete param;
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessAuthToken, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::powerOnDesktop(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData/* = NULL*/)
{
    if(NULL == pchUuid)
    {
        LOG_ERR("%s","NULL == pchUuid");
        return -1;
    }
    ST_ACCESS_DESK_CONTROL_JOBID_IN* param = new(ST_ACCESS_DESK_CONTROL_JOBID_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_POWERON_DESKTOP;
    param->stParamCommon.pCSession = this;

    param->strUuid = pchUuid;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessPowerOnDesktop(param);
        if(NULL == pControlJobData)
        {
            pControlJobData = new(DESK_CONTROL_JOBID_DATA);
        }
        pControlJobData->strJobId = param->stDeskControlJobId.strJobId;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessPowerOnDesktop, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::rebootDesktop(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData/* = NULL*/)
{
    if(NULL == pchUuid)
    {
        LOG_ERR("%s","NULL == pchUuid");
        return -1;
    }
    ST_ACCESS_DESK_CONTROL_JOBID_IN* param = new(ST_ACCESS_DESK_CONTROL_JOBID_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_REBOOT_DESKTOP;
    param->stParamCommon.pCSession = this;

    param->strUuid = pchUuid;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessRebootDesktop(param);
        if(NULL == pControlJobData)
        {
            pControlJobData = new(DESK_CONTROL_JOBID_DATA);
        }
        pControlJobData->strJobId = param->stDeskControlJobId.strJobId;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        /*It is UNBLOCK*/
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessRebootDesktop, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::poweroffDesktop(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData/* = NULL*/)
{
    if(NULL == pchUuid)
    {
        LOG_ERR("%s","NULL == pchUuid");
        return -1;
    }
    ST_ACCESS_DESK_CONTROL_JOBID_IN* param = new(ST_ACCESS_DESK_CONTROL_JOBID_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_POWEROFF_DESKTOP;
    param->stParamCommon.pCSession = this;

    param->strUuid = pchUuid;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessPowerOffDesktop(param);
        if(NULL == pControlJobData)
        {
            pControlJobData = new(DESK_CONTROL_JOBID_DATA);
        }
        pControlJobData->strJobId = param->stDeskControlJobId.strJobId;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessPowerOffDesktop, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::checkRemoteDesktopState(PARAM_SESSION_IN& st_param_in, const char* pchUuid, CHECK_DESK_STATE_DATA* pCheckDeskStateData/* = NULL*/)
{
    if(NULL == pchUuid)
    {
        LOG_ERR("%s","NULL == pchUuid");
        return -1;
    }
    ST_ACCESS_CHECK_DESK_STATE* param = new(ST_ACCESS_CHECK_DESK_STATE);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_CHECK_REMOTEDESKTOP_STATE;
    param->stParamCommon.pCSession = this;

    param->str_deskUuid = pchUuid;
    param->str_SessionTicket = getSessionTicket();
    st_param_in.taskUuid = TASK_UUID_NULL;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessRemoteDesktopState(param);
        if(NULL == pCheckDeskStateData)
        {
            pCheckDeskStateData = new(CHECK_DESK_STATE_DATA);
        }
        *pCheckDeskStateData = param->stCheckDeskState;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessRemoteDesktopState, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::getResParam(const char *resourceUuid, const int resourceType, RESOURCE_PARAMETERS *resourceParameters)
{
    if(NULL == resourceUuid)
    {
        LOG_ERR("%s","NULL == resourceUuid");
        return -1;
    }
    if(NULL != resourceParameters)
    {
        memset(resourceParameters, 0, sizeof(RESOURCE_PARAMETERS));
    }
    return VAccessGetResourceParameters(&m_stNetwork, getSessionTicket(), resourceUuid, resourceType,\
                                 resourceParameters);
}

int CSession::getResParam(PARAM_SESSION_IN& st_param_in, const char *resourceUuid, const int resourceType,\
                              GET_RESOURCE_PARAMETER* st_getResParamOut)
{
    if(NULL == resourceUuid)
    {
        LOG_ERR("%s","NULL == resourceUuid");
        return -1;
    }
    if(st_param_in.isBlock == BLOCKED)
    {
        if(NULL == st_getResParamOut)
        {
            st_getResParamOut = new GET_RESOURCE_PARAMETER;
        }
        int iRet =  getResParam(resourceUuid, resourceType, &(st_getResParamOut->stResPara));
        strcpy(st_getResParamOut->deskUuid, resourceUuid);
        return iRet;
    }

    GET_RES_PARAM_IN* param = new(GET_RES_PARAM_IN);
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;

    param->stParamCommon.type = TYPE_GET_RESPARAMTER;
    param->stParamCommon.pCSession = this;
    param->str_SessionTicket = getSessionTicket();
    param->strResUuid = resourceUuid;
    st_param_in.taskUuid = TASK_UUID_NULL;

    param->iResType = resourceType;

    m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessGetResourceParam, param, &(st_param_in.taskUuid));
    param->stParamCommon.taskUuid = st_param_in.taskUuid;
    return 0;
}

int CSession::getMonitorsInfo(PARAM_SESSION_IN &st_param_in, LIST_MONITORS_INFO *pListMonitorsInfo)
{
    ST_ACCESS_GET_MONITORSINFO *param  = new (ST_ACCESS_GET_MONITORSINFO);
    memset(param, 0, sizeof(ST_ACCESS_GET_MONITORSINFO));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    param->stParamCommon.type = TYPE_GET_MONITORSINFO;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessGetMonitorsInfo(param);
        LOG_INFO("monitor number:\t%d", param->stListMonitorsInfo.monitorNum);
        if(NULL == pListMonitorsInfo)
        {
           pListMonitorsInfo= new (LIST_MONITORS_INFO );
        }

        pListMonitorsInfo->stMonitorInfoList = param->stListMonitorsInfo.stMonitorInfoList;

        pListMonitorsInfo->monitorNum = param->stListMonitorsInfo.monitorNum;
        pListMonitorsInfo->errorcode = param->stListMonitorsInfo.errorcode;

        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessGetMonitorsInfo, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::connectMonitor(PARAM_SESSION_IN &st_param_in, CONNECT_MONITOR *connectMonitor)
{
    ST_ACCESS_CONNECT_MONITOR *param = new (ST_ACCESS_CONNECT_MONITOR);
    memset(param, 0, sizeof(ST_ACCESS_CONNECT_MONITOR));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_CONNECT_MONITOR;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->logonTicket ,getSessionTicket());
     param->mode = connectMonitor->mode;
     param->monitor = connectMonitor->monitor;
     strcpy(param->toMonitorIp ,connectMonitor->toMonitorIp);
    if( param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessConnectMonitor(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessConnectMonitor, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::disconnectMonitor(PARAM_SESSION_IN &st_param_in, DISCONNECT_MONITOR *disconnectMonitor)
{
    ST_ACCESS_DISCONNECT_MONITOR *param = new (ST_ACCESS_DISCONNECT_MONITOR);
    memset(param, 0, sizeof(ST_ACCESS_DISCONNECT_MONITOR));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    param->stParamCommon.type = TYPE_DISCONNECT_MONITOR;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
     strcpy(param->logonTicket ,getSessionTicket());
    param->monitor = disconnectMonitor->monitor;
    if( param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessDisConnectMonitor(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessDisConnectMonitor, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::getUserOrganizations(PARAM_SESSION_IN &st_param_in, USERORGANIZATIONS *userOrganizations )
{
    ST_ACCESS_GET_USERORGANIZATIONS *param = new ST_ACCESS_GET_USERORGANIZATIONS;
    memset(param, 0, sizeof(ST_ACCESS_GET_USERORGANIZATIONS));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessgetUserOrganizations(param);
        if(NULL == userOrganizations)
        {
            userOrganizations = new USERORGANIZATIONS;
        }
        userOrganizations->organizations = param->userorganizations.organizations;
        strcpy(userOrganizations->username,getUSER_INFO().username);
        userOrganizations->errorcode = param->stParamCommon.iErrCode;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessgetUserOrganizations, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::addOrganization(PARAM_SESSION_IN &st_param_in, ADD_ORGANIZATION *addorganization, ADD_ORGANIZATION_DATA *addorganizationData)
{
    ST_ACCESS_ADD_ORGANIZATION *param = new ST_ACCESS_ADD_ORGANIZATION;
    memset(param, 0, sizeof(ST_ACCESS_ADD_ORGANIZATION));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->logonTicket ,getSessionTicket());
    strcpy(param->parentUniqueName, addorganization->parentUniqueName);
    strcpy(param->name, addorganization->name);
    strcpy(param->description, addorganization->description);
    if( param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessaddOrganization(param);
        if( NULL == addorganizationData)
        {
            addorganizationData = new ADD_ORGANIZATION_DATA;
        }
        strcpy(addorganizationData->id, param->addorganizationData.id);
        strcpy(addorganizationData->uniquename, param->addorganizationData.uniquename);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessaddOrganization, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::deleteOrganization(PARAM_SESSION_IN &st_param_in, DELETE_ORGANIZATION *deleteorganization)
{
    ST_ACCESS_DELETE_ORGANIZATION *param = new ST_ACCESS_DELETE_ORGANIZATION;
    memset(param, 0, sizeof(ST_ACCESS_DELETE_ORGANIZATION));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->logonTicket ,getSessionTicket());
    strcpy(param->uniqueName, deleteorganization->uniqueName);
    if( param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessdeleteOrganization(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessdeleteOrganization, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::updateOrganization(PARAM_SESSION_IN &st_param_in, UPDATE_ORGANIZATION *updateorganization)
{
    ST_ACCESS_UPDATE_ORGANIZATION *param = new ST_ACCESS_UPDATE_ORGANIZATION;
    memset(param, 0, sizeof(ST_ACCESS_UPDATE_ORGANIZATION));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->logonTicket ,getSessionTicket());
    strcpy(param->parentUniqueName, updateorganization->parentUniqueName);
    strcpy(param->name, updateorganization->name);
    strcpy(param->description, updateorganization->description);
    strcpy(param->uniqueName, updateorganization->uniqueName);
    if( param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessupdateOrganization(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessupdateOrganization, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::moveOrganizationUsers(PARAM_SESSION_IN &st_param_in, MOVE_ORGANIZATION_USERS *moveorganizationusers)
{
    ST_ACCESS_MOVE_ORGANIZATION_USERS *param = new ST_ACCESS_MOVE_ORGANIZATION_USERS;
    memset(param, 0 , sizeof(ST_ACCESS_MOVE_ORGANIZATION_USERS));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->logonTicket ,getSessionTicket());
    strcpy(param->deleteOld, moveorganizationusers->deleteOld);
    strcpy(param->newUniqueName,moveorganizationusers->newUniqueName);
    strcpy(param->oldUniqueName, moveorganizationusers->oldUniqueName);
    param->users = moveorganizationusers->users;

    if( param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessmoveOrganizationUsers(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessmoveOrganizationUsers, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::addOrganizationUsers(PARAM_SESSION_IN &st_param_in, ADD_ORGANIZATION_USERS *addOrganizationusers)
{
    ST_ACCESS_ADD_ORGANIZATION_USERS *param = new ST_ACCESS_ADD_ORGANIZATION_USERS;
     memset(param, 0 , sizeof(ST_ACCESS_ADD_ORGANIZATION_USERS));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->logonTicket ,getSessionTicket());
    strcpy(param->uniqueName, addOrganizationusers->uniqueName);
    param->users = addOrganizationusers->users;

    if( param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessaddOrganizationUsers(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessaddOrganizationUsers, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::deleteOrganizationUsers(PARAM_SESSION_IN &st_param_in, DELETE_ORGANIZATION_USERS *deleteorganizationusers)
{
    ST_ACCESS_DELETE_ORGANIZATION_USERS *param = new ST_ACCESS_DELETE_ORGANIZATION_USERS;
     memset(param, 0 , sizeof(ST_ACCESS_DELETE_ORGANIZATION_USERS));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->logonTicket ,getSessionTicket());
    strcpy(param->deleteOld, deleteorganizationusers->deleteOld);
    strcpy(param->uniqueName,deleteorganizationusers->uniqueName);
    param->users = deleteorganizationusers->users;

    if( param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessdeleteOrganizationUsers(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }
        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessdeleteOrganizationUsers, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::getOrganizationDetail(PARAM_SESSION_IN &st_param_in, GET_ORGANIZATION_DETAIL *getorganizationDetailparam,ORGANIZATION_DETAIL *organizationdetailData)
{
    ST_ACCESS_GET_ORGANIZATION_DETAIL *param = new ST_ACCESS_GET_ORGANIZATION_DETAIL;
     memset(param, 0 , sizeof(ST_ACCESS_GET_ORGANIZATION_DETAIL));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->uniqueName, getorganizationDetailparam->uniqueName);
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessgetOrganizationDetail(param);
        if(NULL == organizationdetailData)
        {
            organizationdetailData = new ORGANIZATION_DETAIL;
        }
       organizationdetailData->errorcode = param->stParamCommon.iErrCode;
       organizationdetailData->organizationDetail = param->organizationDetail.organizationDetail;

        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessgetOrganizationDetail, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::getRoles(PARAM_SESSION_IN &st_param_in, GET_ROLES *roles)
{
    ST_ACCESS_GET_ROLES *param = new ST_ACCESS_GET_ROLES;
     memset(param, 0 , sizeof(ST_ACCESS_GET_ROLES));
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessgetRoles(param);
        if(NULL == roles)
        {
            roles = new GET_ROLES;
        }
       roles->errorcode = param->stParamCommon.iErrCode;
       roles->roles = param->roles.roles;

        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessgetRoles, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::addRole(PARAM_SESSION_IN &st_param_in, ADD_ROLE *addrole, ADD_ROLE_DATA *addroledata)
{
    ST_ACCESS_ADD_ROLE *param = new ST_ACCESS_ADD_ROLE;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->roleName, addrole->roleName);
    strcpy(param->weight, addrole->weight);
    strcpy(param->description, addrole->description);
    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessaddRole(param);
        if(NULL == addrole)
        {
            addrole = new ADD_ROLE;
        }
       addroledata->errorcode = param->stParamCommon.iErrCode;
       strcpy(addroledata->id, param->addroleData.id);

        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessaddRole, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::deleteRole(PARAM_SESSION_IN &st_param_in, DELETE_ROLE *deleterole)
{
    ST_ACCESS_DELETE_ROLE *param = new ST_ACCESS_DELETE_ROLE;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->roleName, deleterole->roleName);

    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessdeleteRole(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessdeleteRole, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::updateRole(PARAM_SESSION_IN &st_param_in, UPDATE_ROLE *updaterole)
{
    ST_ACCESS_UPDATE_ROLE *param = new ST_ACCESS_UPDATE_ROLE;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->roleName, updaterole->roleName);
    strcpy(param->newRoleName, updaterole->newRoleName);
    strcpy(param->weight, updaterole->weight);
    strcpy(param->description, updaterole->description);

    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessupdateRole(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessupdateRole, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::addroletousers(PARAM_SESSION_IN &st_param_in, ADDROLE_TOUSERS *addroletousers)
{
    ST_ACCESS_ADDROLE_TOUSERS *param = new ST_ACCESS_ADDROLE_TOUSERS;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->roleName, addroletousers->roleName);
    param->users = addroletousers->users;

    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessaddroletousers(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessaddroletousers, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::deleterolefromusers(PARAM_SESSION_IN &st_param_in, DELETEROLE_FROMUSERS *deleterolefromusers)
{
    ST_ACCESS_DELETEROLE_FROMUSERS *param = new ST_ACCESS_DELETEROLE_FROMUSERS;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->roleName, deleterolefromusers->roleName);
    param->users = deleterolefromusers->users;

    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessdeleterolefromusers(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessdeleterolefromusers, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::getuserprivileges(PARAM_SESSION_IN &st_param_in, GET_USERPRIVILEGES_PARAM *get_userprivileges_param, GET_USERPRIVILEGES_DATA *get_userprivileges_data )
{
    ST_ACCESS_GET_USERPRIVILEGES_PARAM *param = new ST_ACCESS_GET_USERPRIVILEGES_PARAM;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->target, get_userprivileges_param->target);
    strcpy(param->targetId, get_userprivileges_param->targetId);
    strcpy(param->domain, get_userprivileges_param->domain);
    strcpy(param->userName, get_userprivileges_param->userName);

    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessgetuserprivileges(param);
        if(get_userprivileges_data == NULL)
        {
            get_userprivileges_data = new GET_USERPRIVILEGES_DATA;
        }
        get_userprivileges_data->privileges = param->userprivilegesData.privileges;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessgetuserprivileges, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::getprivileges(PARAM_SESSION_IN &st_param_in, GET_PRIVILEGES_PARAM *get_privileges_param, GET_PRIVILEGES_DATA *get_privileges_data)
{
    ST_ACCESS_GET_PRIVILEGES_PARAM *param = new ST_ACCESS_GET_PRIVILEGES_PARAM;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->target, get_privileges_param->target);
    strcpy(param->targetId, get_privileges_param->targetId);
    strcpy(param->owner, get_privileges_param->owner);
    strcpy(param->ownerId, get_privileges_param->ownerId);

    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessgetprivileges(param);
        if(get_privileges_data == NULL)
        {
            get_privileges_data = new GET_PRIVILEGES_DATA;
        }
        get_privileges_data->privileges = param->privilegesData.privileges;
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessgetprivileges, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::addprivileges(PARAM_SESSION_IN &st_param_in, ADD_PRIVILEGES *addprivileges)
{
    ST_ACCESS_ADD_PRIVILEGES *param = new ST_ACCESS_ADD_PRIVILEGES;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->target, addprivileges->target);
    strcpy(param->targetId, addprivileges->targetId);
    strcpy(param->owner, addprivileges->owner);
    strcpy(param->ownerId, addprivileges->ownerId);
    param->privileges = addprivileges->privileges;

    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessaddprivileges(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessaddprivileges, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::deleteprivileges(PARAM_SESSION_IN &st_param_in, DELETE_PRIVILEGES *deleteprivileges)
{
    ST_ACCESS_DELETE_PRIVILEGES *param = new ST_ACCESS_DELETE_PRIVILEGES;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    strcpy(param->target, deleteprivileges->target);
    strcpy(param->targetId, deleteprivileges->targetId);
    strcpy(param->owner, deleteprivileges->owner);
    strcpy(param->ownerId, deleteprivileges->ownerId);
    param->privileges = deleteprivileges->privileges;

    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessdeleteprivileges(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessdeleteprivileges, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::setSeatNumbers(PARAM_SESSION_IN &st_param_in, SEATNUMBERS *seatList )
{
    ST_ACCESS_SETSEATNUMBERS *param = new ST_ACCESS_SETSEATNUMBERS;
    param->stParamCommon.network = m_stNetwork;
    param->stParamCommon.callback = &callback_csession;
    param->stParamCommon.callback_ui = st_param_in.callbackFun;
    param->stParamCommon.callbackParam = st_param_in.callback_param;
    param->stParamCommon.bIsBlocked = st_param_in.isBlock;
    strcpy(param->logonTicket ,getSessionTicket());
    st_param_in.taskUuid = TASK_UUID_NULL;
    param->stParamCommon.pCSession = this;
    param->seatList = seatList->seatlist;

    if(param->stParamCommon.bIsBlocked == BLOCKED)
    {
        VAccessSetSeatNumbers(param);
        if(NULL != st_param_in.callback_param)
        {
            st_param_in.callback_param->errorCode = param->stParamCommon.iErrCode;
        }

        delete(param);
    }
    else
    {
        m_threadPool.createThread(param->stParamCommon.type, (FUN_THREAD)VAccessSetSeatNumbers, param, &(st_param_in.taskUuid));
        param->stParamCommon.taskUuid = st_param_in.taskUuid;
    }
    return 0;
}

int CSession::testConnectState()
{
    ST_TEST_CONNECTION_STATE param;
    param.stParamCommon.network = m_stNetwork;
    param.stParamCommon.callback = NULL;
    param.stParamCommon.callback_ui = NULL;
    param.stParamCommon.callbackParam = NULL;
    param.stParamCommon.bIsBlocked = BLOCKED;

    return VAccessTestConnectState(&param);
}

int releaseCSession(void* pvoid )
{
    CSession* pSession =(CSession*) pvoid;
    if(NULL != pSession)
    {
        delete pSession;
    }
    pSession = NULL;
//    LOG_INFO("%s","delete pSession returned...");
    return 0;
}










//int CSession::getAppList(char* uuid, APP_LIST& app_list)
//{
//    map<string, APP_LIST>::const_iterator iter = m_mapAppList.find(uuid);
//    if(m_mapAppList.end() == iter)
//    {
//        return -1;
//    }
//    app_list = iter->second;
//    return 0;
//}

//int CSession::updateAppList(char* uuid, const APP_LIST& app_list)
//{
//    APP_LIST app_list_get;
//    int iRet = getAppList(uuid, app_list_get);
//    if(iRet>=0)
//    {
//        m_mapAppList.erase(uuid);
//    }
//    m_mapAppList.insert(map<string, APP_LIST>::value_type(uuid, app_list));
//    return 0;
//}

//int CSession::updateAppList(const map<string, APP_LIST> mapAppList)
//{
//    clearAppList();
//    for(map<string, APP_LIST>::const_iterator iter = m_mapAppList.begin(); iter != m_mapAppList.end(); iter++)
//    {
//        m_mapAppList.insert(map<string, APP_LIST>::value_type(iter->first, iter->second));
//    }
//    return 0;
//}

//int CSession::removeAppList(char* uuid)
//{
//    m_mapAppList.erase(uuid);
//    return 0;
//}

//int CSession::clearAppList()
//{
//    m_mapAppList.clear();
//    return 0;
//}

//case TYPE_STOP_DESKTOPPOOL:
//{
//    ST_ACCESS_DESK_CONTROL_JOBID_IN* param = (ST_ACCESS_DESK_CONTROL_JOBID_IN*)pvoid;
//    int iUserRequestStop = 0;
//    CSession* pSession = param->stParamCommon.pCSession;
//    if(NULL != pSession)
//    {
//        pSession->m_threadPool.threadFinished(param->stParamCommon.taskUuid, &iUserRequestStop);
//        if(NULL != param->stParamCommon.callback_ui)
//        {
//            if(NULL != param->stParamCommon.callbackParam)
//                param->stParamCommon.callbackParam->errorCode = param->stParamCommon.iErrCode;
//            DESK_CONTROL_JOBID_DATA* pJobId = new(DESK_CONTROL_JOBID_DATA);
//            pJobId->strJobId = param->stDeskControlJobId.strJobId;
//            if(iUserRequestStop == 0)
//                (*(param->stParamCommon.callback_ui))(param->stParamCommon.callbackParam, param->stParamCommon.type, (void*)pJobId);
//        }
//    }
//    delete param;
//    param = NULL;
//    break;
//}
