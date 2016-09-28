#ifndef DS_ACCESS_H
#define DS_ACCESS_H

#include "ds_vclient.h"
#include "ds_session.h"

#define REQUEST_DOMAIN_LIST     "/RestService/User/DomainList"
#define REQUEST_GET_USERNAME    "/RestService/OccupyDesktop"
#define REQUEST_GET_USER_INFO   "/RestService/User/GetUserInfo"
#define REQUEST_GET_USER_RES    "/RestService/User/ListUserResource"
#define REQUEST_LAUNCH_RES      "/RestService/LaunchResource"
#define REQUEST_SHUTDOWN_RES    "/RestService/ShutdownResource"

#define REQUEST_CHANGE_NTUSER   "/RestService/User/User"
#define REQUEST_CHANGE_USER     "/RestService/User/UpdatePassword"
#define REQUEST_START_DESKPOOL  "/RestService/DesktopPool/StartDesktopPool"
#define REQUEST_RESTART_DESPOOL "/RestService/DesktopPool/RestartDesktopPool"
#define REQUEST_STOP_DESKPOOL   "/RestService/DesktopPool/StopDesktopPool"
#define REQUEST_DESKPOOL_STATE  "/RestService/DesktopPool/DesktopPoolState"
#define REQUEST_DESKTOP_INFO    "/RestService/RequestDesktop"

#define REQUEST_TERMINAL_DESKTOP_CTL "/RestService/ManageTerminals"

#define REQUEST_GET_MONITORSINFO "/RestService/GetMonitorsInfo"
#define REQUEST_CONNECT_MONITOR "/RestService/ConnectMonitor"
#define REQUEST_DISCONNECT_MONITOR "/RestService/DisconnectMonitor"

#define REQUEST_GET_USERORGANIZATIONS "/RestService/GetUserOrganizations"
#define REQUEST_ADD_ORGANIZATION "/RestService/AddOrganization"
#define REQUEST_DELETE_ORGANIZATION "/RestService/DeleteOrganization"
#define REQUEST_UPDATE_ORGANIZATION "/RestService/UpdateOrganization"
#define REQUEST_MOVE_ORGANIZATIONUSERS "/RestService/MoveOrganizationUsers"
#define REQUEST_ADD_ORGANIZATIONUSERS "/RestService/AddOrganizationUsers"
#define REQUEST_DELETE_ORGANIZATIONUSERS "/RestService/DeleteOrganizationUsers"
#define REQUEST_GET_ORGANIZATIONDETAIL "/RestService/GetOrganizationDetail"
#define REQUEST_GET_ROLES "/RestService/GetRoles"
#define REQUEST_ADD_ROLE "/RestService/AddRole"
#define REQUEST_DELETE_ROLE "/RestService/DeleteRole"
#define REQUEST_UPDATE_ROLE "/RestService/UpdateRole"
#define REQUEST_ADDROLE_TOUSERS "/RestService/AddRoleToUsers"
#define REQUEST_DELETEROLE_FROMUSERS "/RestService/DeleteRoleFromUsers"

#define REQUEST_GET_USERPRIVILEGES "/RestService/GetUserPrivileges"
#define REQUEST_GET_PRIVILEGES "/RestService/GetPrivileges"
#define REQUEST_ADD_PRIVILEGES "/RestService/AddPrivileges"
#define REQUEST_DELETE_PRIVILEGES "/RestService/DeletePrivileges"
#define REQUEST_SET_SEATNUMBERS "/RestService/SetSeatNumber"


#define REQUEST_ATTACH_VDISK    "/RestService/AttachVirtualDisk"
#define REQUEST_ATTACH_VDISK_TODESKTOP    "/RestService/AttachVirtualDiskToDesktop"
#define REQUEST_ASYN_QUERY      "/RestService/QueryAsyncJobResult"
#define REQUEST_GET_CLIENT_VER  "/RestService/Client/GetClientVersion"
#define REQUEST_OPEN_CHANNEL    "/RestService/OpenChannel"
#define REQUEST_CLOSE_CHANNEL   "/RestService/CloseChannel"

#define REQUEST_POWER_DESK      "/RestService/RemoteDesktop/PowerOnDesktop"
#define REQUEST_REBOOT_DESK     "/RestService/RemoteDesktop/RebootDesktop"
#define REQUEST_POWEROFF_DESK   "/RestService/RemoteDesktop/PowerOffDesktop"
#define REQUEST_REMOTEDEST_STATE    "/RestService/RemoteDesktop/RemoteDesktopState"
#define REQUEST_LOGIN_SESSION   "/Action/LoginSession"
#define REQUEST_GET_TERMINAL_FUNC   "/RestService/GetTerminalFunc"

#define REQUEST_START_CLASS        "/RestService/StartClass"
#define REQUEST_END_CLASS    "/RestService/EndClass"
#define REQUEST_START_BROADCAST_SCREEN    "/RestService/StartBroadcastScreen"
#define REQUEST_END_BROADCAST_SCREEN    "/RestService/EndBroadcastScreen"

#define REQUEST_KEEP_SESSION    "/Action/KeepSession"
#define REQUEST_LOGOUT_SESSION  "/Action/LogoutSession"
#define REQUEST_AUTH_TOKEN      "/Action/TokenAuth"
#define REQUEST_TEST_CONNECTION_STATE   "/bin/com/VServer/assets/rightObliqueSize.gif"

typedef int (*pCallBackSession)(void* , int);
typedef pCallBackSession CALL_BACK_FUN_CESSION;
class CSession;
struct AccessInCommon
{
    NETWORK network;
    int bIsBlocked;
    CALL_BACK_FUN_CESSION callback;
    CALL_BACK_FUN_UI callback_ui;
    CALLBACK_PARAM_UI* callbackParam;
    int iErrCode;
    int type;
    CSession* pCSession;
    taskUUID taskUuid;
//    int iCallUiCallbackFun; //used to specify whether need to call callback_ui function
};
typedef AccessInCommon ST_ACCESS_IN_COMMON;

struct AccessGetDomainIn
{
    /*
     *description:
     *  used in VAccessGetDomainList function
     *parameter:
     *  stParamCommon(ST_ACCESS_IN_COMMON)[in]:
     *      ...
     *  stDomainData(DOMAIN_DATA)[out]:
     *      contains domains got
     **/
    ST_ACCESS_IN_COMMON stParamCommon;
    DOMAIN_DATA stDomainData;
};
typedef struct AccessGetDomainIn ST_ACCESS_GETDOMAIN_IN;

class CSession;

struct AccessGetUsername{
    ST_ACCESS_IN_COMMON stParamCommon;
    char uuid[512];
    char userName[512];
};
typedef struct AccessGetUsername ST_ACCESS_GET_USERNAME;

struct AccessLoginIn
{
    /*
     *description:
     *  used in VAccessLoginSession function
     *parameter:
     *  stParamCommon(ST_ACCESS_IN_COMMON)[in]:
     *      ...
     *  stUserInfo(USER_INFO)[in]:
     *      contains user infos
     *  stLoginData(LOGIN_DATA)[out]:
     *      contains login info got(including session ticket)
     **/
    ST_ACCESS_IN_COMMON stParamCommon;
    USER_INFO stUserInfo;
    LOGIN_DATA stLoginData;
    bool bAutoLogin;
};
typedef struct AccessLoginIn ST_ACCESS_LOGIN_IN;

struct AccessKeepSession
{
    /*
     *description:
     *  used in VAccessKeepSession function
     *parameter:
     *  stParamCommon(ST_ACCESS_IN_COMMON)[in]:
     *      ...
     *  str_SessionTicket(string)[in]:
     *      session ticket
     *  st_keepSession(KEEP_SESSION_DATA)[out]:
     *      contains session timeout  interval gots
     **/
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    KEEP_SESSION_DATA st_keepSession;
};
typedef struct AccessKeepSession ST_ACCESS_KEEPSESSION_IN;

struct AccessLogoutIn
{
    /*
     *description:
     *  used in VAccessLogoutSession function
     *parameter:
     *  stParamCommon(ST_ACCESS_IN_COMMON)[in]:
     *      ...
     *  str_SessionTicket(string)[in]:
     *      session ticket
     **/
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
};
typedef struct AccessLogoutIn ST_ACCESS_LOGOUT_IN;

struct AccessGetUserInfoIn
{
    /*
     *description:
     *  used in VAccessGetUserInfo function
     *parameter:
     *  stParamCommon(ST_ACCESS_IN_COMMON)[in]:
     *      ...
     *  str_SessionTicket(string)[in]:
     *      session ticket
     *  st_GetuserInfo(GET_USER_INFO_DATA)[out]:
     *      session ticket
     **/
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    GET_USER_INFO_DATA st_GetuserInfo;
};
typedef struct AccessGetUserInfoIn ST_ACCESS_GETUSERINFO_IN;

struct AccessGetIcon
{
    /*
     *description:
     *  used in VAccessGetIcon function
     *parameter:
     *  stParamCommon(ST_ACCESS_IN_COMMON)[in]:
     *      ...
     *  strUrlPath(string)[in]
     *      ...
     **/
    ST_ACCESS_IN_COMMON stParamCommon;
    string strUrlPath;
    GET_ICON_DATA stGetIcon;
};
typedef struct AccessGetIcon ST_ACCESS_GETICON_IN;

struct AccessListRes
{
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    bool b_getResParam;
    LIST_USER_RESOURCE_DATA sListUserRes;
};
typedef struct AccessListRes ST_ACCESS_LISTRES_IN;


//struct AccessGetResParam
//{
//    ST_ACCESS_IN_COMMON stParamCommon;
//    string str_SessionTicket;
//    string strResUuid;
//    int iResType;
//    GET_RESOURCE_PARAMETER stGetResPara;
//};
//typedef struct AccessGetResParam ST_ACCESS_GETRESPARAM_IN;

struct AccessLaunchRes
{
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    string strResUuid;
    int iResType;
    int iDisplayProtocol;
    LAUNCH_RESOURCE_DATA stLaunchResData;
};
typedef struct AccessLaunchRes ST_ACCESS_LAUNCHRES_IN;

struct AccessShutdownRes
{
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    string str_resTicket;
    int iIsRelease; //new added 2013 0723
};
typedef struct AccessShutdownRes ST_ACCESS_SHUTDOWN_RES_IN;

struct AccessChangeUserInfo
{
    ST_ACCESS_IN_COMMON stParamCommon;
    USER_INFO stUserInfo;
    NT_ACCOUNT_INFO stAccountInfo;
    string strPasswd;
    string strNewPasswd;
    string strNtUserName;
    string strNtPasswd;
};
typedef struct AccessChangeUserInfo ST_ACCESS_CHANGE_USER_INFO_IN;

struct AccessRequestDesktop
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    string Cpu;
    string Memory;
    string Os;
    string Disk;
    string desktopDescrip;

};
typedef struct AccessRequestDesktop ST_ACCESS_REQUEST_DESKTOP;


struct AccessTerminalDesktopCtl
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char loginTicket[MAX_LEN];
    vector<string> uuid;
    string msg;

};
typedef struct AccessTerminalDesktopCtl ST_ACCESS_TERMINAL_DESKTOP_CTL;

struct AccessGetMonitorsInfo
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    LIST_MONITORS_INFO stListMonitorsInfo;
};
typedef struct AccessGetMonitorsInfo ST_ACCESS_GET_MONITORSINFO;
struct AccessConnectMonitor
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    int monitor;
    int mode;
    char toMonitorIp[MAX_LEN];
};
typedef AccessConnectMonitor ST_ACCESS_CONNECT_MONITOR;
struct AccessDisconnectMonitor
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    int monitor;
};
typedef struct AccessDisconnectMonitor ST_ACCESS_DISCONNECT_MONITOR;


struct AccessGetUserOrganizations
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    USERORGANIZATIONS userorganizations;
};
typedef struct AccessGetUserOrganizations ST_ACCESS_GET_USERORGANIZATIONS;

struct AccessAddOrganization
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char parentUniqueName[MAX_LEN];
    char name[MAX_LEN];
    char description[MAX_LEN];
    ADD_ORGANIZATION_DATA addorganizationData;
};
typedef struct AccessAddOrganization ST_ACCESS_ADD_ORGANIZATION;

struct AccessDeleteOrganization
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char uniqueName[MAX_LEN];
};
typedef struct AccessDeleteOrganization ST_ACCESS_DELETE_ORGANIZATION;

struct AccessUpdateOrganization
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char parentUniqueName[MAX_LEN];
    char uniqueName[MAX_LEN];
    char name[MAX_LEN];
    char description[MAX_LEN];
};
typedef struct AccessUpdateOrganization ST_ACCESS_UPDATE_ORGANIZATION;

struct AccessMoveOrganizationUsers
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char oldUniqueName[MAX_LEN];
    char newUniqueName[MAX_LEN];
    char deleteOld[MAX_LEN];
    std::vector<Userinfo> users;
};
typedef struct AccessMoveOrganizationUsers ST_ACCESS_MOVE_ORGANIZATION_USERS;

struct AccessAddOrganizationUsers
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char uniqueName[MAX_LEN];
    std::vector<Userinfo> users;
};
typedef struct AccessAddOrganizationUsers ST_ACCESS_ADD_ORGANIZATION_USERS;

struct AccessDeleteOrganizationUsers
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char uniqueName[MAX_LEN];
    char deleteOld[MAX_LEN];
    std::vector<Userinfo> users;
};
typedef struct AccessDeleteOrganizationUsers ST_ACCESS_DELETE_ORGANIZATION_USERS;

struct AccessGetOrganizationDetail
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char uniqueName[MAX_LEN];
    ORGANIZATION_DETAIL organizationDetail;
};
typedef AccessGetOrganizationDetail ST_ACCESS_GET_ORGANIZATION_DETAIL;

struct AccessGetRoles
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    GET_ROLES roles;
};
typedef struct AccessGetRoles ST_ACCESS_GET_ROLES;

struct AccessAddRole
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char roleName[MAX_LEN];
    char weight[MAX_LEN];
    char description[MAX_LEN];
    ADD_ROLE_DATA addroleData;
};
typedef struct AccessAddRole ST_ACCESS_ADD_ROLE;

struct AccessDeleteRole
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char roleName[MAX_LEN];
};
typedef struct AccessDeleteRole ST_ACCESS_DELETE_ROLE;

struct AccessUpdateRole
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char roleName[MAX_LEN];
    char newRoleName[MAX_LEN];
    char weight[MAX_LEN];
    char description[MAX_LEN];
};
typedef struct AccessUpdateRole ST_ACCESS_UPDATE_ROLE;

struct AccessRole_Users
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char roleName[MAX_LEN];
    std::vector<Userinfo> users;
};
typedef struct AccessRole_Users ST_ACCESS_ADDROLE_TOUSERS;
typedef struct AccessRole_Users ST_ACCESS_DELETEROLE_FROMUSERS;

struct AccessGetUserPrivilegesParam
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char userName[MAX_LEN];
    char domain[MAX_LEN];
    char target[MAX_LEN];
    char targetId[MAX_LEN];
    GET_USERPRIVILEGES_DATA userprivilegesData;
};
typedef AccessGetUserPrivilegesParam ST_ACCESS_GET_USERPRIVILEGES_PARAM;

struct AccessGetPrivilegesParam
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char owner[MAX_LEN];
    char ownerId[MAX_LEN];
    char target[MAX_LEN];
    char targetId[MAX_LEN];
    GET_PRIVILEGES_DATA privilegesData;
};
typedef AccessGetPrivilegesParam ST_ACCESS_GET_PRIVILEGES_PARAM;


struct AccessOperatePrivileges
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    char owner[MAX_LEN];
    char ownerId[MAX_LEN];
    char target[MAX_LEN];
    char targetId[MAX_LEN];
    std::vector<USERPRIVILEGESDATA> privileges;
};
typedef struct AccessOperatePrivileges ST_ACCESS_ADD_PRIVILEGES;
typedef struct AccessOperatePrivileges ST_ACCESS_DELETE_PRIVILEGES;
struct AccessSetSeatNumbers
{
    ST_ACCESS_IN_COMMON stParamCommon;
    char logonTicket[MAX_LEN];
    std::vector<SEATNUMBER> seatList;
};
typedef struct AccessSetSeatNumbers ST_ACCESS_SETSEATNUMBERS;

struct AccessDeskControl
{
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    string strUuid;
    DESK_CONTROL_JOBID_DATA stDeskControlJobId;
};
typedef struct AccessDeskControl ST_ACCESS_DESK_CONTROL_JOBID_IN;


struct AccessCheckDeskState
{
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    string str_deskUuid;
    int iResourceType;
    CHECK_DESK_STATE_DATA stCheckDeskState;
};
typedef struct AccessCheckDeskState ST_ACCESS_CHECK_DESK_STATE;

struct AccessAttachVDisk
{
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    string str_desktopUuid;
    int iDesktopType;
    ATTACH_VDISK_DATA stAttachVDisk;
    string devicePath;
};
typedef struct AccessAttachVDisk ST_ACCESS_ATTACH_VDISK;

struct AccessAsynQuery
{
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    string strJobId;
    ASYN_QUERY_DATA stAsynQuery;

};
typedef struct AccessAsynQuery ST_ACCESS_ASYN_QUERY;

struct AccessQueryClientVersion
{
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    int iClientType;
    QUERY_VERSION_DATA stQueryVersion;
};
typedef struct AccessQueryClientVersion ST_ACCESS_QUERY_CLIENT_VERSION;

struct AccessChannelOp
{
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    string str_ip;
    int port;
    CHANNEL_OP_DATA st_channel_op;
};
typedef struct AccessChannelOp ST_CHANNEL_OP;

struct AccessAuthToken
{
    ST_ACCESS_IN_COMMON stParamCommon;
    USER_INFO stUserInfo;
    LOGIN_DATA_TOKEN stLoginData;
};
typedef struct AccessAuthToken ST_AUTH_TOKEN;

struct GetResParam
{
    ST_ACCESS_IN_COMMON stParamCommon;
    string str_SessionTicket;
    string strResUuid;
    int iResType;
    GET_RESOURCE_PARAMETER stResParm_out;
};

typedef struct GetResParam GET_RES_PARAM_IN;

struct TestConnectionState
{
    ST_ACCESS_IN_COMMON stParamCommon;
    //string str_SessionTicket;
};
typedef struct TestConnectionState ST_TEST_CONNECTION_STATE;

#endif //DS_ACCESS_H
