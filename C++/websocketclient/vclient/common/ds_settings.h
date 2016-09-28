#ifndef DS_SETTINGS_H
#define DS_SETTINGS_H
#include "ds_vclient.h"
enum BarSatus{UNKNOWN_BAR_STATUS = 0, HASBAR_STATE, NOBAR_STATE};
typedef enum BarSatus RDP_HAS_BAR;
typedef enum BarSatus FAP_HAS_BAR;

enum DiskMapStatus{UNKNOWN_MAP_STATUS = 0, NOMAP_STATE, MAP_STATE};
typedef enum DiskMapStatus DISK_MAP_STATUS;

enum UpdateSetting{UNKNOWN_UPDATING_STATUS = 0, AUTO_DETECT, MANUAL_UPDATE, AUTO_UPDATE};
typedef enum UpdateSetting UPDATE_SETTINGS;

enum LounchAppOnSysStart{UNKNOWN_LOUCH_APP_ON_SYS_STATUS = 0, LOUNCH_ON_START, NOT_LOUNCH_ON_START};
typedef LounchAppOnSysStart LOUCH_APP_ON_SYS_START;

namespace Ui {
    class SettingWindow;
}
class TitleWidget;
class CSession;
class CMutexOp;
class QProcess;

struct Settings
{
    DISK_MAP_STATUS m_mapset;
    FILEINFOLIST m_mapFilePathList;
    NETWORK  m_network;
    RDP_HAS_BAR m_rdpBar;
    FAP_HAS_BAR m_fapBar;
    UPDATE_SETTINGS m_updateSetting;
    LOUCH_APP_ON_SYS_START m_louchAppOnSysStart;
    int iAutoConnectToServer; //1.yes  0.no    in net cmd ui(to decide wether to show network setting dlg when launching vClient)
};

//username passwd current-selected-domain
typedef Settings SETTINGS_VCLIENT;

struct LoginSettings
{
    USER_INFO stUserInfo;
    int iRemember;
    int iAutoLogin;
    int iAttachVDisk;
};
typedef struct LoginSettings SETTINGS_LOGIN;

struct DefaultDesktop
{
    char  uuid[MAX_LEN];
    char appName[MAX_LEN];
    char userName[MAX_LEN];
    char serverIp[MAX_LEN];
    //int isLoadvDisk;
    int isMapUsb;
    int connectProtocal;
    int isAutoConnect;
};
typedef struct DefaultDesktop SETTING_DEFAULTAPP;

#endif // DS_SETTINGS_H
