#ifndef DS_LAUNCHAPP_H
#define DS_LAUNCHAPP_H
#include "../common/ds_settings.h"
#include "../common/cthread.h"
#include "../common/ds_session.h"

 #define TYPE_FRONTEND_LAUNCHDESKTOP                  10000
#define TYPE_FRONTEND_DISCONN_APP                    10001
//error code
#define ERROR_CODE_LAUNCHPROC_QUIT_210               -10210

#define ERROR_CODE_LAUNCHPROC_RDP_TIME_OUT_9         -10009

struct LaunchRdp
{
    /*
     *description:
     *  this structure is used when launch desktop using rpd protocol.
     *parameter:
     *  stAppInfo(APP_LIST)[in]:
     *      the desktop information
     *  stPort(SERIAL_PARALLEL_PORT)[in]:
     *      the port needs to map
     *  barStatus(RDP_HAS_BAR)[in]:
     *      whether to show floater when the desktop has been launched
     *  bAttachDisk(bool)[in]:
     *       needs to attach disk if true == bAttachDisk     *
     **/
    APP_LIST stAppInfo;
    SERIAL_PARALLEL_PORT stPort;
    RDP_HAS_BAR barStatus;
    bool bAttachDisk;
    bool bMapUsb;
    bool bMapFileSystem;
    bool isConnected;
    VIRTUALDISK launchDisk;
};
typedef struct LaunchRdp LAUCH_RDP;

struct LaunchFAP
{
    /*
     *description:
     *  this structure is used when launch desktop using fap protocol.
     *parameter:
     *  stAppInfo(APP_LIST)[in]:
     *      the desktop information
     *  barStatus(RDP_HAS_BAR)[in]:
     *      whether to show floater when the desktop has been launched
     *  bAttachDisk(bool)[in]:
     *       needs to attach disk if true == bAttachDisk
     **/
    APP_LIST stAppInfo;
        FAP_HAS_BAR barStatus;
        SERIAL_PARALLEL_PORT stPort;
        VIRTUALDISK launchDisk;
        bool bAttachDisk;
        bool bMapUsb;
        bool isConnected;
        bool bMapFileSystem;
        DISK_MAP_STATUS mapStatus;

        FILEINFOLIST stFileInfo;
};
typedef struct LaunchFAP LAUCH_FAP;

struct LaunchTERMINAL
{
    /*
     *description:
     *  this structure is used when launch desktop using fap protocol.
     *parameter:
     *  stAppInfo(APP_LIST)[in]:
     *      the desktop information
     *  barStatus(RDP_HAS_BAR)[in]:
     *      whether to show floater when the desktop has been launched
     *  bAttachDisk(bool)[in]:
     *       needs to attach disk if true == bAttachDisk
     **/
    APP_LIST stAppInfo;
    FAP_HAS_BAR barStatus;
    bool bAttachDisk;
    bool bMapUsb;
};
typedef struct LaunchTERMINAL LAUNCH_TERMINAL;

/*
 *description:
 *      used to identify what kind of protocol is used when launching a desktop
 *      LAUNCH_TYPE_RDP:    use rdp protocol
 *      LAUNCH_TYPE_FAP:    use fap protocol
 *      LAUNCH_TYPE_TERMINAL: terminal desktop
 **/
enum LaunchType{ LAUNCH_TYPE_RDP = 0, LAUNCH_TYPE_FAP, LAUNCH_TYPE_TERMINAL};
typedef enum LaunchType LAUNCH_TYPE;

enum ReloadDiskType{ ONLY_RETURN = -1, RELOAD_VDISK_NO = 0, RELOAD_VDISK_YES};
typedef enum ReloadDiskType RELOAD_VDISK_TYPE;


struct ParamLaunchCommonIn
{
    /*
     *description:
     *  this structure contains information needed when launch desktop using fap or protocol.
     *parameter:
     *  pFunUi(CALL_BACK_FUN_UI)[in]:
     *      the call back function pointer. it is necessary only when isBlock==UNBLOCK
     *  pCallbackUi_param(CALLBACK_PARAM_UI)[in]:
     *      this parameter will be pass to the callback function. if isBlock==BLOCKED,
     *      it is not used
     *  isBlock(int)[in]:
     *      if BLOCKED == isBlock, it runs in block mode, else it runs in a thread,
     *      the result will be send in the callback function
     **/
    CALL_BACK_FUN_UI pFunUi;
    CALLBACK_PARAM_UI* pCallbackUi_param;
    int isBlock;
};
typedef struct ParamLaunchCommonIn PARAM_LAUNCH_COMMON_IN;

struct LaunchDesktopData
{
    /*
     *description:
     *  this structure contains information that passed to the call back function.
     *parameter:
     *  strDesktopUuid(string):
     *      desktop uuid
     **/
    string strDesktopUuid;
    APP_LIST stAppInfo;
    int iOpType;
};
typedef struct LaunchDesktopData LAUNCH_DESKTOP_DATA;

struct DisConnApp
{
    string strDesktopUuid;
    CALL_BACK_FUN_UI pFunUi;
    CALLBACK_PARAM_UI* pCallbackUi_param;
    int isBlock;
};
typedef struct DisConnApp DISCONN_APP;

//////////////////////////////////
#ifdef WIN32
#define APP_NAME_FAP            "fap/bin/fap.exe"
#define APP_NAME_RDP            "vcfap/vcfap.exe"
#define APP_FUSB_TCP_PATH       "fap/fusb_windows_tcp.exe"

#define MAIL_SLOT_NAME_VCLIENT          "\\\\.\\mailslot\\vclient_mailslot"
#define MAIL_SLOT_NAME_FUSB             "\\\\.\\mailslot\\fusb_mailslot"
#else
#define APP_NAME_RDP            "/usr/bin/frdp"
#define APP_NAME_FAP            "/usr/bin/fap"
#define APP_FUSB_TCP_PATH       "/usr/bin/tcp-usbip"
#define APP_NAME_TERMINAL  "/usr/bin/italc"
#endif
//forward declaration
class CFrontEndBase;
class CLaunchApp;

struct LaunchThreadInfo
{
    /*
     *description:
     *  this structure contains information about a thread which runs a destop.
     *parameter:
     *  hd(THREAD_HANDLE):
     *      thread handle.
     *  pApp(CFrontEndBase*):
     *      a pointer to the CFrontEnd*(manage all the infos needed to launch a
     *      desktop) instance.
     *  type(LAUNCH_TYPE):
     *      used to identify what kind of protocol is used when launching a desktop
     *  iAppExited(int):
     *      used to identify whether the thread has exited.
     *      true: the thread has finished.
     **/
    THREAD_HANDLE hd;
    CFrontEndBase* pApp;
    LAUNCH_TYPE type;
    int iAppExited; //0 no 1 yes
};
typedef struct LaunchThreadInfo LAUNCH_THREAD_INFO;

///*defines a call back function*/
typedef int (*pCallBack)(int type, int, void*, void* );
typedef pCallBack CALL_BACK_LAUNCH;
///*define some macro to identify the reason to call the callback function*/
#define TYPE_QUERYVACCESS_FAILED                    15005
#define TYPE_LAUNCH_FORM_CMDPARAM_FAILED            15006
#define TYPE_LAUNCHAPP_CREATE_PROC_FINISHED         15010
#define TYPE_LAUNCH_WAITPROC_RETURNED               15015
#define TYPE_LAUNCH_SHUTDOWN_RES_FINISHED           15020
#define TYPE_LAUNCH_ATTACH_DISK_FAILED              15025
#define TYPE_LAUNCH_ATTACH_DISK_SUCCEED             15030
#define TYPE_BEGIN_ATTACH_DISK                      15035
#define TYPE_DESKTOP_NOT_AVAILABLE                  15040

struct LaunchCallBackInfo
{
    /*
     *description:
     *  this structure contains information about a thread which runs a destop.
     *parameter:
     *  pFunUi(CALL_BACK_FUN_UI):
     *      the call back function pointer. it is necessary only when isBlock==UNBLOCK
     *  pCallbackUi_param(CALLBACK_PARAM_UI):
     *      this parameter will be pass to the callback function. if isBlock==BLOCKED,
     *      it is not used
     *  pFun(CALL_BACK_LAUNCH):
     *      the call back function pointer.
     *  pLaunchApp(CLaunchApp*):
     *      the CLaunchApp class instance
     *  type(LAUNCH_TYPE):
     *      used to identify what kind of protocol is used when launching a desktop
     **/
    CALL_BACK_FUN_UI pFunUi;
    CALLBACK_PARAM_UI* pCallbackUi_param;
    CALL_BACK_LAUNCH pFun;
    CLaunchApp* pLaunchApp;
    LAUNCH_TYPE type;
};
typedef struct LaunchCallBackInfo LAUNCH_CALLBACK_INFO;

struct DisconnAppThreadInfo
{
    DISCONN_APP stDisconnApp;
    CLaunchApp* pLaunchApp;
};
typedef struct DisconnAppThreadInfo DISCONN_APP_THREAD_INFO;

struct DisConnectAppData //call back function return data
{
    /*
     *description:
     *  this structure contains information that passed to the call back function.
     *parameter:
     *  strDesktopUuid(string):
     *      desktop uuid
     **/
    string strDesktopUuid;
};
typedef struct DisConnectAppData DISCONNECT_APP_DATA;

#endif // DS_LAUNCHAPP_H
