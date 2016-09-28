#ifndef DS_SESSION_H
#define DS_SESSION_H

#include <string>
#include <vector>
#include "ds_vclient.h"
using namespace std;

#define TYPE_GETDOMAIN                  1
#define TYPE_LOGIN                      5
#define TYPE_LOGOUT                     10

#define TYPE_SWITCH_ACCESS      11

#define TYPE_KEEPSESSION                15
#define TYPE_GETUSERINFO                20

#define TYPE_GETICON                    25
#define TYPE_LIST_USER_RES              30
#define TYPE_LAUNCH_RES                 40
#define TYPE_SHUTDOWN_RES               45

#define TYPE_CHANGE_USER_INFO           50
#define TYPE_START_DESKTOPPOOL          55
#define TYPE_RESTART_DESKTOPPOOL        60
#define TYPE_STOP_DESKTOPPOOL           65
#define TPPE_CHECK_DESKTOPPOOL_STATE    70

#define TYPE_ATTACH_DISK                75
#define TYPE_QUERY_ASYN_JOB_RESULT      80
#define TYPE_QUERY_CLIENT_VERSION       85
#define TYPE_OPEN_CHANNEL               90
#define TYPE_CLOSE_CHANNEL              95

#define TYPE_AUTH_TOKEN                 100
#define TYPE_POWERON_DESKTOP            105
#define TYPE_REBOOT_DESKTOP             110
#define TYPE_POWEROFF_DESKTOP           115
#define TYPE_CHECK_REMOTEDESKTOP_STATE  120

#define TYPE_GET_RESPARAMTER            125

#define TYPE_REQUEST_DESKTOP            155
#define TYPE_CHANGE_NTUSER_INFO         160

#define TYPE_POWEROFF_TERMINAL        165
#define TYPE_RESTART_TERMINAL           170
#define TYPE_MSG_TERMINAL                 175

#define TYPE_GET_MONITORSINFO      180
#define TYPE_CONNECT_MONITOR      185
#define TYPE_DISCONNECT_MONITOR 190
#define TYPE_GET_USERNAME 195
#define BLOCKED 1
#define UNBLOCK 0

#define TASK_UUID_NULL 0

typedef int taskUUID;
enum LOGIN_TYPE{LOGIN_TYPE_UNKNOWN, LOGIN_TYPE_USERNAME_PASSWD, LOGIN_TYPE_TOKEN_AUTH};


///!!!!!!!!!!!if you change the structure of the struct, please pay attention to
//claunchapp.cpp launchCallBack function's case:TYPE_LAUNCH_ATTACH_DISK_FAILED
//you should check whether the operator = (*pParamUi = *(pCallBackInfo->pCallbackUi_param);) is suitable
struct CallBackParamUi
{
    void *pUi;
    int uiType;
    char appUuid[MAX_LEN];
    int errorCode;
};
typedef struct CallBackParamUi CALLBACK_PARAM_UI;

typedef int (*pCallBackUi)(CALLBACK_PARAM_UI*, int, void* );
typedef pCallBackUi CALL_BACK_FUN_UI;

struct ParamSessionIn
{
    /*
     *description:
     *  this structure is used when calling CSession member functions.
     *parameter:
     *  callbackFun(CALL_BACK_FUN_UI)[in]:
     *      the call back function pointer. it is necessary only when isBlock==UNBLOCK
     *  isBlock(int)[in]:
     *      if BLOCKED == isBlock, it runs in block mode, else it runs in a thread,
     *      the result will be send in the callback function
     *  callback_param(CALLBACK_PARAM_UI)[in]:
     *      this parameter will be pass to the callback function. if isBlock==BLOCKED,
     *      it is not used
     *  taskUuid(taskUUID)[out]:
     *      if isBlock==UNBLOCK, this parameters contains a handle used to cancle the
     *      request
     *
     **/
    CALL_BACK_FUN_UI callbackFun;
    int isBlock;
    CALLBACK_PARAM_UI* callback_param;
    taskUUID taskUuid;
};
typedef struct ParamSessionIn PARAM_SESSION_IN;

struct ParamLaunchResIn
{
    string strResUuid;
    int iResType;
    int iDisplayProtocol;
};
typedef struct ParamLaunchResIn PARAM_LAUNCH_RES_IN;

struct ParamChangeUserInfoIn
{
    string strPasswd;
    string strNewPasswd;
    string strNtUserName;
    string strNtPasswd;
};
typedef struct ParamChangeUserInfoIn PARAM_CHANGE_USER_INFO_IN;

//struct ParamChannelOpIn
//{
//    string str_ip;
//    int iPort;
//};
//typedef struct ParamChannelOpIn PARAM_CHANNELOP_IN;

struct DomainData
{
    /*
     *description:
     *
     *parameter:
     *  vstrDomainlists(vector<string>):
     *      contains a list of domains
     **/
    vector<string> vstrDomainlists;
};
typedef struct DomainData DOMAIN_DATA;
struct UsernameData{
    char userName[512];
};
typedef struct UsernameData USERNAME_DATA;
struct LoginData
{
    /*
     **description:
     *
     *parameter:
     *  stLoginInfo(NT_ACCOUNT_INFO):
     *      contains login info
     **/
    NT_ACCOUNT_INFO stLoginInfo;
    NETWORK stNetwork;
};
typedef struct LoginData LOGIN_DATA;
typedef struct LoginData LOGIN_DATA_TOKEN;

struct LogoutData
{
    int iErrorCode;
};
typedef struct LogoutData LOGOUT_DATA;

struct KeepSessionData
{
    int i_timeOut;
};
typedef struct KeepSessionData KEEP_SESSION_DATA;

struct GetUserInfoData
{
    vector<VIRTUALDISK> vstVirtualDisks;
    NT_ACCOUNT_INFO stNtAccountInfo;
};
typedef struct GetUserInfoData GET_USER_INFO_DATA;


struct GetIconData
{//ATTENTION: the caller need to release data(call if(NULL!= data)free(data))
    char* data;
    int iLen;
    std::string strAppUuid;
};
typedef struct GetIconData GET_ICON_DATA;

struct ListUserResource
{
    vector<APP_LIST> stAppList;
    vector<APP_LIST> stAppBakList;

};
typedef struct ListUserResource LIST_USER_RESOURCE_DATA;

struct MonitorInfo
{
    int monitor;
    int status;
    int mode;
    char username[MAX_LEN];
    char domain[MAX_LEN];
};
typedef MonitorInfo MONITOR_INFO;

struct ListMonitorsInfo
{
    int monitorNum;
    int errorcode;
    vector<MONITOR_INFO> stMonitorInfoList;
};

typedef struct ListMonitorsInfo LIST_MONITORS_INFO;


struct ConnectMonitor
{
    char logonTicket[MAX_LEN];
    int monitor;
    int mode;
    char toMonitorIp[MAX_LEN];
};

typedef struct ConnectMonitor CONNECT_MONITOR;

struct DisconnectMonitor
{
    char logonTicket[MAX_LEN];
    int monitor;
};
typedef struct DisconnectMonitor DISCONNECT_MONITOR;

struct Organization
{
    char id[MAX_LEN];
    char name[MAX_LEN];
    char uniqueName[MAX_LEN];
    char description[MAX_LEN];
    std::vector<Userinfo> users;
};
typedef struct Organization ORGANIZATION;

struct UserOrganizations
{
    int errorcode;
    char username[512];
    vector<ORGANIZATION> organizations;
};
typedef struct UserOrganizations USERORGANIZATIONS;

struct AddOrganization
{
    char logonTicket[MAX_LEN];
    char parentUniqueName[MAX_LEN];
    char name[MAX_LEN];
    char description[MAX_LEN];
};
typedef struct AddOrganization ADD_ORGANIZATION;

struct AddOrganizationData
{
    char id[MAX_LEN];
    char uniquename[MAX_LEN];
};
typedef struct AddOrganizationData ADD_ORGANIZATION_DATA;

struct DeleteOrganization
{
    char logonTicket[MAX_LEN];
    char uniqueName[MAX_LEN];
};
typedef struct DeleteOrganization DELETE_ORGANIZATION;

struct UpdateOrganization
{
    char logonTicket[MAX_LEN];
    char parentUniqueName[MAX_LEN];
    char uniqueName[MAX_LEN];
    char name[MAX_LEN];
    char description[MAX_LEN];
};
typedef struct UpdateOrganization UPDATE_ORGANIZATION;

struct MoveOrganizationUsers
{
    char logonTicket[MAX_LEN];
    char oldUniqueName[MAX_LEN];
    char newUniqueName[MAX_LEN];
    char deleteOld[MAX_LEN];
    std::vector<Userinfo> users;
};
typedef struct MoveOrganizationUsers MOVE_ORGANIZATION_USERS;

struct AddOrganizationUsers
{
    char logonTicket[MAX_LEN];
    char uniqueName[MAX_LEN];
    std::vector<Userinfo> users;
};
typedef struct AddOrganizationUsers ADD_ORGANIZATION_USERS;

struct DeleteOrganizationUsers
{
    char logonTicket[MAX_LEN];
    char uniqueName[MAX_LEN];
    char deleteOld[MAX_LEN];
    std::vector<Userinfo> users;
};
typedef struct DeleteOrganizationUsers DELETE_ORGANIZATION_USERS;

struct OrganizationDetail
{
    int errorcode;
    std::vector<ORGANIZATION> organizationDetail;
};
typedef OrganizationDetail ORGANIZATION_DETAIL;

struct GetOrganizationDetail
{
    char logonTicket[MAX_LEN];
    char uniqueName[MAX_LEN];
};
typedef struct GetOrganizationDetail GET_ORGANIZATION_DETAIL;

struct Role
{
    char id[MAX_LEN];
    char name[MAX_LEN];
    char weight[MAX_LEN];
    char description[MAX_LEN];
};
typedef struct Role ROLE;

struct GetRoles
{
    int errorcode;
    std::vector<ROLE> roles;
};
typedef struct GetRoles GET_ROLES;

struct AddRole
{
    char logonTicket[MAX_LEN];
    char roleName[MAX_LEN];
    char weight[MAX_LEN];
    char description[MAX_LEN];
};
typedef struct AddRole ADD_ROLE;

struct AddRoleData
{
    int errorcode;
    char id[MAX_LEN];
};
typedef struct AddRoleData ADD_ROLE_DATA;

struct DeleteRole
{
    char logonTicket[MAX_LEN];
    char roleName[MAX_LEN];
};
typedef struct DeleteRole DELETE_ROLE;
struct UpdateRole
{
    char logonTicket[MAX_LEN];
    char roleName[MAX_LEN];
    char newRoleName[MAX_LEN];
    char weight[MAX_LEN];
    char description[MAX_LEN];
};
typedef struct UpdateRole UPDATE_ROLE;

struct Role_Users
{
    char logonTicket[MAX_LEN];
    char roleName[MAX_LEN];
    std::vector<Userinfo> users;
};
typedef struct Role_Users ADDROLE_TOUSERS;
typedef struct Role_Users DELETEROLE_FROMUSERS;

struct GetUserPrivilegesParam
{
    char logonTicket[MAX_LEN];
    char userName[MAX_LEN];
    char domain[MAX_LEN];
    char target[MAX_LEN];
    char targetId[MAX_LEN];
};
typedef GetUserPrivilegesParam GET_USERPRIVILEGES_PARAM;

struct UserPrivilegesData
{
    char action[MAX_LEN];
    char transmission[MAX_LEN];
};
typedef struct UserPrivilegesData USERPRIVILEGESDATA;

struct GetUserPrivilegesData
{
    int errorcode;
    std::vector<USERPRIVILEGESDATA> privileges;
};
typedef struct GetUserPrivilegesData GET_USERPRIVILEGES_DATA;

struct GetPrivilegesParam
{
    char logonTicket[MAX_LEN];
    char owner[MAX_LEN];
    char ownerId[MAX_LEN];
    char target[MAX_LEN];
    char targetId[MAX_LEN];
};
typedef GetPrivilegesParam GET_PRIVILEGES_PARAM;


struct GetPrivilegesData
{
    int errorcode;
    std::vector<USERPRIVILEGESDATA> privileges;
};
typedef struct GetPrivilegesData GET_PRIVILEGES_DATA;

struct OperatePrivileges
{
    char logonTicket[MAX_LEN];
    char owner[MAX_LEN];
    char ownerId[MAX_LEN];
    char target[MAX_LEN];
    char targetId[MAX_LEN];
    vector<USERPRIVILEGESDATA> privileges;
};
typedef struct OperatePrivileges ADD_PRIVILEGES;
typedef struct OperatePrivileges DELETE_PRIVILEGES;

struct SeatNumber
{
    char desktopname[512];
    char number[512];
};
typedef struct SeatNumber SEATNUMBER;
struct SeatNumbers
{
    vector<SEATNUMBER> seatlist;
};
typedef struct SeatNumbers SEATNUMBERS;

struct GetResourceParameter
{
    char deskUuid[MAX_LEN];
    RESOURCE_PARAMETERS stResPara;
};
typedef struct GetResourceParameter GET_RESOURCE_PARAMETER;

struct LaunchResource
{
    RESOURCE_INFO stResInfo;
};
typedef struct LaunchResource LAUNCH_RESOURCE_DATA;

struct DeskControlJobId
{
    string strJobId;
};
typedef struct DeskControlJobId DESK_CONTROL_JOBID_DATA;
typedef struct DeskControlJobId ATTACH_VDISK_DATA;

struct CheckDeskState
{
    int iState;
    int iRdpState;
    string strIp;
    string strJobId;    //uuid
};
typedef struct CheckDeskState CHECK_DESK_STATE_DATA;


struct AsyncQuery
{
    int iJobStatus;
};
typedef struct AsyncQuery ASYN_QUERY_DATA;


struct QueryVersion
{
    string str_ClientVersion;
};
typedef struct QueryVersion QUERY_VERSION_DATA;

struct ChannelOp
{
    string str_ip;
    int iPort;
    char usb_Uuid[MAX_LEN];
};
typedef struct ChannelOp CHANNEL_OP_DATA;
typedef struct ChannelOp PARAM_CHANNELOP_IN;

struct RequestDesktop
{
    char logonTicket[MAX_LEN];
    string Cpu;
    string Memory;
    string Os;
    string Disk;
    string desktopDescrip;
};
typedef struct RequestDesktop REQUEST_DESKTOP;

struct TerminalDesktop
{
    char loginTicket[MAX_LEN];
    std::vector<string> uuid;
    string msg;
};

typedef struct TerminalDesktop TERMINAL_DESKTOP;



#endif // DS_SESSION_H
