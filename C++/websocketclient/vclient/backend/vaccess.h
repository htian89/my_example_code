#ifndef VACCESS_H
#define VACCESS_H

#include "../common/ds_access.h"

int VAccessGetDomainList(ST_ACCESS_GETDOMAIN_IN* param);
int VAccessGetUserName(ST_ACCESS_GET_USERNAME *param);
int VAccessLoginSession(ST_ACCESS_LOGIN_IN* param);

/*2014-11-5 add*/
int VAccessGetTerminalFunc(ST_ACCESS_LOGIN_IN* param);

/*2014-11-21 */
int VAccessStartClass(ST_ACCESS_LOGIN_IN * param);
int VAccessEndClass(ST_ACCESS_LOGIN_IN* param);
int VAccessStartBroadcastScreen(ST_ACCESS_LOGIN_IN* param);
int VAccessEndBroadcastScreen(ST_ACCESS_LOGIN_IN* param);
void * ServerTeacherForFap(void * arg);
void server_for_websocket(void);

int VAccessKeepSession(ST_ACCESS_KEEPSESSION_IN *param);
int VAccessLogoutSession(ST_ACCESS_LOGOUT_IN* param);

int VAccessSwitchAccessSession(ST_ACCESS_LOGOUT_IN* param);

int VAccessGetUserInfo(ST_ACCESS_GETUSERINFO_IN* param);

int VAccessGetIcon(ST_ACCESS_GETICON_IN* param);
int VAccessListUserResource(ST_ACCESS_LISTRES_IN* param);
int VAccessGetResourceParameters(const NETWORK *network, const char *logonTicket,
                                 const char *resourceUuid, const int resourceType,
                                 RESOURCE_PARAMETERS *resourceParameters);
int VAccessGetResourceParam(GET_RES_PARAM_IN* param);
int VAccessLaunchResource(ST_ACCESS_LAUNCHRES_IN* param);

int VAccessShutdownResource(ST_ACCESS_SHUTDOWN_RES_IN* param);

int VAccessRequestDesktop(ST_ACCESS_REQUEST_DESKTOP* param);
int VAccessGetMonitorsInfo(ST_ACCESS_GET_MONITORSINFO* param);
int VAccessConnectMonitor(ST_ACCESS_CONNECT_MONITOR* param);
int VAccessDisConnectMonitor(ST_ACCESS_DISCONNECT_MONITOR *param);

int VAccessgetUserOrganizations(ST_ACCESS_GET_USERORGANIZATIONS *param);
int VAccessaddOrganization(ST_ACCESS_ADD_ORGANIZATION *param);
int VAccessdeleteOrganization(ST_ACCESS_DELETE_ORGANIZATION *param);
int VAccessupdateOrganization(ST_ACCESS_UPDATE_ORGANIZATION *param);
int VAccessmoveOrganizationUsers(ST_ACCESS_MOVE_ORGANIZATION_USERS *param);
int VAccessaddOrganizationUsers(ST_ACCESS_ADD_ORGANIZATION_USERS *param);
int VAccessdeleteOrganizationUsers(ST_ACCESS_DELETE_ORGANIZATION_USERS *param);
int VAccessgetOrganizationDetail(ST_ACCESS_GET_ORGANIZATION_DETAIL *param);
int VAccessgetRoles(ST_ACCESS_GET_ROLES *param);
int VAccessaddRole(ST_ACCESS_ADD_ROLE *param);
int VAccessdeleteRole(ST_ACCESS_DELETE_ROLE *param);
int VAccessupdateRole(ST_ACCESS_UPDATE_ROLE *param);
int VAccessaddroletousers(ST_ACCESS_ADDROLE_TOUSERS *param);
int VAccessdeleterolefromusers(ST_ACCESS_DELETEROLE_FROMUSERS *param);

int VAccessgetuserprivileges(ST_ACCESS_GET_USERPRIVILEGES_PARAM *param);
int VAccessgetprivileges(ST_ACCESS_GET_PRIVILEGES_PARAM *param);
int VAccessaddprivileges(ST_ACCESS_ADD_PRIVILEGES *param);
int VAccessdeleteprivileges(ST_ACCESS_DELETE_PRIVILEGES *param);


int VAccessSetSeatNumbers(ST_ACCESS_SETSEATNUMBERS *param);

int VAccessPoweroffTerminalDesktop(ST_ACCESS_TERMINAL_DESKTOP_CTL* param);
int VAccessRestartTerminalDesktop(ST_ACCESS_TERMINAL_DESKTOP_CTL* param);
int VAccessMsgTerminalDesktop(ST_ACCESS_TERMINAL_DESKTOP_CTL* param);

int VAccessChangeUserInfo(ST_ACCESS_CHANGE_USER_INFO_IN* param);
int VAccessChangeNtUserInfo(ST_ACCESS_CHANGE_USER_INFO_IN* param);
int VAccessStartDesktopPool(ST_ACCESS_DESK_CONTROL_JOBID_IN* param);
int VAccessRestartDesktopPool(ST_ACCESS_DESK_CONTROL_JOBID_IN* param);
int VAccessStopDesktopPool(ST_ACCESS_DESK_CONTROL_JOBID_IN* param);
int VAccessCheckDesktopPoolState(ST_ACCESS_CHECK_DESK_STATE* param);



int VAccessAttachVirtualDisk(ST_ACCESS_ATTACH_VDISK* param);// not used
int VAccessAttachVirtualDiskToDesktop(ST_ACCESS_ATTACH_VDISK* param);
int VAccessQueryAsyncJobResult(ST_ACCESS_ASYN_QUERY* param);
int VAccessQueryClientVersion(ST_ACCESS_QUERY_CLIENT_VERSION* param);
int VAccessOpenChannel(ST_CHANNEL_OP* param);
int VAccessCloseChannel(ST_CHANNEL_OP* param);

//int VAccessAttachVirtualDisk(const NETWORK *network,
//        const char *logonTicket, const char *desktopPoolUuid, char *jobId);
//int VAccessQueryAsyncJobResult(const NETWORK *network,
//        const char *logonTicket, const char *jobId, int *jobStatus);
//int VAccessQueryClientVersion(const NETWORK * network,
//        const int vClientType, char * clientVersion);
//int VAccessOpenChannel(const NETWORK *network, const char *logonTicket,
//                       const char *ip, char *return_ip, int *port);
//int VAccessCloseChannel(const NETWORK *network, const char *logonTicket,
//                        const char *ip, int *port);

int VAccessAuthToken(ST_AUTH_TOKEN* param);
int VAccessPowerOnDesktop(ST_ACCESS_DESK_CONTROL_JOBID_IN* param);
int VAccessRebootDesktop(ST_ACCESS_DESK_CONTROL_JOBID_IN* param);
int VAccessPowerOffDesktop(ST_ACCESS_DESK_CONTROL_JOBID_IN* param);
int VAccessRemoteDesktopState(ST_ACCESS_CHECK_DESK_STATE* param);


//currently not support unblock mode
int VAccessTestConnectState(ST_TEST_CONNECTION_STATE* param);

//int VAccessAuthToken(const NETWORK *network,
//                     const USER_INFO *userInfo,
//                      NT_ACCOUNT_INFO *login);
//int VAccessPowerOnDesktop(const NETWORK *network,
//                          const char *logonTicket, const char *remoteDesktopUuid, char *jobId);
//int VAccessRebootDesktop(const NETWORK *network,
//                         const char *logonTicket, const char *remoteDesktopUuid, char *jobId);
//int VAccessPowerOffDesktop(const NETWORK *network,
//                           const char *logonTicket, const char *remoteDesktopUuid, char *jobId);
//int VAccessRemoteDesktopState(const NETWORK *network,
//                              const char *logonTicket, const char *remoteDesktopUuid,
//                              int *rdpState, int *vmState, char *ip);

    //    int VAccessGetApplicationList(const NETWORK *network, const char *logonTicket,
    //			VAccessApplication *apps, int *appCount);
    //    int VAccessGetResourceList(const NETWORK *network, const char *logonTicket,
    //			VAccessApplication *apps, int *appCount, VAccessDesktopPool *desktopPools,
    //			int *dpCount, VIRTUALDISK *virtualDisks, int *diskCount,
    //			VAccessRemoteDesktop *remoteDesktops, int *rdCount);

    //    int VAccessLaunchApplication(const NETWORK *network,
    //            const char *logonTicket, const char *appUuid, SELECTAPPLICATION *selectApp);
    //    int VAccessLaunchDesktopPool(const NETWORK *network, const char *logonTicket,
    //            const char *desktopPoolUuid, SELECTAPPLICATION *selectApp);
    //    int VAccessShutdownApplication(const NETWORK *network,
    //			const char *logonTicket, const char *appTicket);
    //    int VAccessShutdownDesktopPool(const NETWORK *network,
    //			const char *logonTicket, const char *desktopPoolTicket);

//char *VAccessGetIcon(const NETWORK *network,
//        const char *urlPath, int *dataLength);
//int VAccessListUserResource(const NETWORK *network,
//                                const char *logonTicket,
//                                APP_LIST *apps, int *appCount,
//                                APP_LIST *desktopPools, int *dpCount,
//                                APP_LIST *remoteDesktops, int *rdCount);
//int VAccessGetResourceParameters(const NETWORK *network, const char *logonTicket,
//                                 const char *resourceUuid, const int resourceType,
//                                 RESOURCE_PARAMETERS *resourceParameters);
//int VAccessLaunchResource(const NETWORK *network, const char *logonTicket,
//                          const char *resourceUuid, const int resourceType,
//                          const int displayProtocol, RESOURCE_INFO *resource);
//int VAccessShutdownResource(const NETWORK *network, const char *logonTicket,
//                            const char *resourceTicket);

//int VAccessChangeUserInfo(const NETWORK *network,
//        const USER_INFO *userInfo, const NT_ACCOUNT_INFO *login,
//        const char *password, const char *ntUsername, const char *ntPassword);
//int VAccessStartDesktopPool(const NETWORK *network,
//        const char *logonTicket, const char *desktopPoolUuid, char *jobId);
//int VAccessRestartDesktopPool(const NETWORK *network,
//        const char *logonTicket, const char *desktopPoolUuid, char *jobId);
//int VAccessStopDesktopPool(const NETWORK *network,
//        const char *logonTicket, const char *desktopPoolUuid, char *jobId);
//int VAccessCheckDesktopPoolState(const NETWORK *network,
//        const char *logonTicket, const char *desktopPoolUuid, int *state, char *ip);



//#ifdef __cplusplus
//}
//#endif

#endif // VACCESS_H
