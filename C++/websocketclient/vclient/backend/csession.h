#ifndef CSESSION_H
#define CSESSION_H
#include <string>
#include <map>
#include "../common/ds_vclient.h"
#include "../common/ds_session.h"
#include "../common/cmutexop.h"
#include "../common/cthread.h"
#include "../common/log.h"
#include "cthreadpool.h"

using namespace std;

int releaseCSession(void* );

/*
 *  setSessionInfo can only be used before loginSession or authToken has called
 *  after calling logoutSession. we suggest you to delete CSession class instance
 **/
class CSession
{
public:
    static CSession* GetInstance()
    {
        m_mutex.lock();
        if (NULL == m_pSessionInst)
        {
//            LOG_INFO("%s","going to  new CSession");
            m_pSessionInst = new CSession();
        }
        m_mutex.unlock();
        return m_pSessionInst;
    }
    static void Release()
    {
//        CSession* pSession = NULL;
//        m_mutex.lock();
//        pSession = m_pSessionInst;
////        if (NULL != m_pSessionInst)
////            delete m_pSessionInst;
//        m_pSessionInst = NULL;
//        m_mutex.unlock();
        CSession* pSession = NULL;
        m_mutex.lock();
        pSession = m_pSessionInst;
        m_pSessionInst = NULL;
        m_mutex.unlock();
        if(NULL != pSession)//release CSession in thread
        {
//            LOG_INFO("%s","going to  release CSession");
            CThread::createThread(NULL, NULL, (FUN_THREAD)(&releaseCSession), (void*)pSession);
        }        
    }

    ~CSession();
public:
    CSession();
    CSession(const NETWORK network, const USER_INFO userinfo);

public:
    int getDomainList(PARAM_SESSION_IN& st_param_in, DOMAIN_DATA* pDomainData = NULL);
    int getUserName(PARAM_SESSION_IN &st_param_in, char uuid[512],USERNAME_DATA *pUserName=NULL);
    int loginSession(PARAM_SESSION_IN& st_param_in, LOGIN_DATA* pLoginData = NULL, bool bAutoLogin = false);
    int logoutSession(PARAM_SESSION_IN& st_param_in);

    int switchAccessSession(PARAM_SESSION_IN& st_param_in);

    int getUserInfo(PARAM_SESSION_IN& st_param_in, GET_USER_INFO_DATA* pGetUserInfo = NULL);

    int getIcon(PARAM_SESSION_IN& st_param_in, char* appUuid, GET_ICON_DATA* pIconData= NULL);
    int listUserResource(PARAM_SESSION_IN& st_param_in, bool bGetResParam = true, LIST_USER_RESOURCE_DATA* pRes= NULL);
    int launchResource(PARAM_SESSION_IN& st_param_in, PARAM_LAUNCH_RES_IN& st_launchRes, LAUNCH_RESOURCE_DATA* pLaunchRes = NULL);
    int shutdownRes(PARAM_SESSION_IN& st_param_in, char* pResTicket, int iIsRelease = 0);

    int changeUserInfo(PARAM_SESSION_IN& st_param_in, PARAM_CHANGE_USER_INFO_IN& st_changeUserInfo);
    int changeNtUserInfo(PARAM_SESSION_IN& st_param_in, PARAM_CHANGE_USER_INFO_IN& st_changeUserInfo);
    int startDesktopPool(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData = NULL);
    int restartDesktopPool(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData = NULL);
    int stopDesktopPool(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData = NULL);
    int checkDesktopPoolState(PARAM_SESSION_IN& st_param_in, const char* pchUuid, CHECK_DESK_STATE_DATA* pCheckDeskStateData = NULL);

    int requestDesktop(PARAM_SESSION_IN& st_param_in, REQUEST_DESKTOP& requestDesktop);
    int powerOffTerminal(PARAM_SESSION_IN& st_param_in, TERMINAL_DESKTOP &terminalDesktop);
    int restartTerminal(PARAM_SESSION_IN& st_param_in, TERMINAL_DESKTOP &terminalDesktop);
    int messageTerminal(PARAM_SESSION_IN& st_param_in, TERMINAL_DESKTOP &TerminalDesktop);


    int attachVDisk(PARAM_SESSION_IN& st_param_in, const char* pchUuid, int desktopType, const VIRTUALDISK& vDisk, ATTACH_VDISK_DATA* pVDiskData = NULL);
    int queryAsynJobResult(PARAM_SESSION_IN& st_param_in, const char* pchUuid, ASYN_QUERY_DATA* pAsynQueryData = NULL);
    int queryClientVersion(PARAM_SESSION_IN& st_param_in, int iClientType, QUERY_VERSION_DATA* pVersionData = NULL);
    int openChannel(PARAM_SESSION_IN& st_param_in, PARAM_CHANNELOP_IN& st_channelData, CHANNEL_OP_DATA* pChannelOpData = NULL);
    int closeChannel(PARAM_SESSION_IN& st_param_in, PARAM_CHANNELOP_IN& st_channelData, CHANNEL_OP_DATA* pChannelOpData = NULL);

    int authToken(PARAM_SESSION_IN& st_param_in, LOGIN_DATA_TOKEN* pAuthTokenData = NULL);
    int powerOnDesktop(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData = NULL);
    int rebootDesktop(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData = NULL);
    int poweroffDesktop(PARAM_SESSION_IN& st_param_in, char* pchUuid, DESK_CONTROL_JOBID_DATA* pControlJobData = NULL);
    int checkRemoteDesktopState(PARAM_SESSION_IN& st_param_in, const char* pchUuid, CHECK_DESK_STATE_DATA* pCheckDeskStateData = NULL);

    int getResParam(const char *resourceUuid, const int resourceType, RESOURCE_PARAMETERS *resourceParameters = NULL);
    int getResParam(PARAM_SESSION_IN& st_param_in, const char *resourceUuid, const int resourceType,\
                    GET_RESOURCE_PARAMETER* st_getResParamOut = NULL);

    int getMonitorsInfo(PARAM_SESSION_IN &st_param_in, LIST_MONITORS_INFO *pListMonitorsInfo = NULL);
    int connectMonitor(PARAM_SESSION_IN &st_param_in, CONNECT_MONITOR *connectMonitor);
    int disconnectMonitor(PARAM_SESSION_IN &st_param_in, DISCONNECT_MONITOR *disconnectMonitor);

    int getUserOrganizations(PARAM_SESSION_IN &st_param_in, USERORGANIZATIONS *userOrganizations );
    int addOrganization(PARAM_SESSION_IN &st_param_in, ADD_ORGANIZATION *addorganization, ADD_ORGANIZATION_DATA *addorganizationData);
    int deleteOrganization(PARAM_SESSION_IN &st_param_in, DELETE_ORGANIZATION *deleteorganization);
    int updateOrganization(PARAM_SESSION_IN &st_param_in, UPDATE_ORGANIZATION *updateorganization);
    int moveOrganizationUsers(PARAM_SESSION_IN &st_param_in, MOVE_ORGANIZATION_USERS *moveorganizationusers);
    int addOrganizationUsers(PARAM_SESSION_IN &st_param_in, ADD_ORGANIZATION_USERS *addOrganizationusers);
    int deleteOrganizationUsers(PARAM_SESSION_IN &st_param_in, DELETE_ORGANIZATION_USERS *deleteorganizationusers);
    int getOrganizationDetail(PARAM_SESSION_IN &st_param_in, GET_ORGANIZATION_DETAIL *getorganizationDetailparam,ORGANIZATION_DETAIL *organizationdetail);
    int getRoles(PARAM_SESSION_IN &st_param_in, GET_ROLES *roles);
    int addRole(PARAM_SESSION_IN &st_param_in, ADD_ROLE *addrole, ADD_ROLE_DATA *addroledata);
    int deleteRole(PARAM_SESSION_IN &st_param_in, DELETE_ROLE *deleterole);
    int updateRole(PARAM_SESSION_IN &st_param_in, UPDATE_ROLE *updaterole);
    int addroletousers(PARAM_SESSION_IN &st_param_in, ADDROLE_TOUSERS *addroletousers);
    int deleterolefromusers(PARAM_SESSION_IN &st_param_in, DELETEROLE_FROMUSERS *deleterolefromusers);

    int getuserprivileges(PARAM_SESSION_IN &st_param_in, GET_USERPRIVILEGES_PARAM *get_userprivileges_param, GET_USERPRIVILEGES_DATA *get_userprivileges_data );
    int getprivileges(PARAM_SESSION_IN &st_param_in, GET_PRIVILEGES_PARAM *get_privileges_param, GET_PRIVILEGES_DATA *get_privileges_data);
    int addprivileges(PARAM_SESSION_IN &st_param_in, ADD_PRIVILEGES *addprivileges);
    int deleteprivileges(PARAM_SESSION_IN &st_param_in, DELETE_PRIVILEGES *deleteprivileges);
    int setSeatNumbers(PARAM_SESSION_IN &st_param_in, SEATNUMBERS *seatList );






    int testConnectState();
    //int createThread(int type);
    //{
    //    switch()
    //}
    LOGIN_TYPE getLoginType(){ return m_loginType;}
    int setLoginType(LOGIN_TYPE loginType){ m_loginType = loginType; return 0;}
    int cancelTask(const taskUUID taskUuid);
    //int getDataFromTaskUUID(const taskUUID taskUuid );
    int setSessionInfo(const NETWORK* const pNetwork, const USER_INFO* const pUserinfo);
    const NT_ACCOUNT_INFO& getNT_ACCOUNT_INFO(){    return m_accountInfo;   }
    const USER_INFO& getUSER_INFO()  {   return m_stUserInfo;    }
    const NETWORK& getNetwork() {   return m_stNetwork;     }

public:
    int setAccountInfo(const NT_ACCOUNT_INFO& acc ) {   m_accountInfo = acc;   return 0; }
    int setUserInfo(const USER_INFO& userInfo)  {   m_stUserInfo = userInfo;    return 0;   }
    int setLoginStatusFailed()      {  m_iHasLogin = 0;  return 0;}
    int setLoginStatusSucceed()      {  m_iHasLogin = 1;  return 0;}
public:
    CThreadPool m_threadPool;

private:

private://operate to private member data
    int setNetwork(const NETWORK& network)  {   m_stNetwork = network;  return 0; }
    const char* getSessionTicket(){ return m_accountInfo.logonTicket;   }
private:
    map<string, APP_LIST> m_mapAppList;
    NETWORK m_stNetwork;
    USER_INFO m_stUserInfo;
    NT_ACCOUNT_INFO m_accountInfo;
    int m_iHasLogin; //0 no 1 yes
    LOGIN_TYPE m_loginType;

public:
    static CSession* volatile m_pSessionInst;
    static CMutexOp m_mutex; // used to get a single instance of Cession

};

int set_pSessionToNULL(void*pvoid, int iType);

#endif // CSESSION_H

//int mycallback1(void* pvoid, int iInt);

//public://operate to private member data
////operate to m_mapAppList
//    int getAppList(char* uuid, APP_LIST& app_list);
//    int updateAppList(char* uuid, const APP_LIST& app_list);
//    int updateAppList(const map<string, APP_LIST> mapAppList);
//    int removeAppList(char* uuid);
//    int clearAppList();
////operate to m_stUserInfo
//    const USER_INFO& getUserInfo()  {   return m_stUserInfo;    }
//    int setUserInfo(const USER_INFO& userInfo)  {   m_stUserInfo = userInfo;    return 0;   }

//private://operate to private member data
////operate to m_stNetwork
//    const NETWORK& getNetwork() {   return m_stNetwork; }
//    int setNetwork(const NETWORK& network)  {   m_stNetwork = network;  return 0; }
