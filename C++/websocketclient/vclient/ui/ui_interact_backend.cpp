#include "ui_interact_backend.h"
#include "userlogindlg.h"
#include "desktoplistdialog.h"
#include "cpersonalsetting.h"
#include "desktopsettingdialog.h"
#include "requestdesktopdlg.h"
#include "../common/errorcode.h"
#include "terminaldesktoplist.h"
#include "autologindialog.h"
#include "../common/ds_launchapp.h"
#include "cmessagebox.h"
#include "../imageconf.h"
#include "common/tcp_message.h"

#include <iostream>
extern AutoLoginDialog *g_autoLoginDlg;
extern char notes[MAX_LEN] ;
extern QTextCodec *codec;

int uiCallBackFunc(CALLBACK_PARAM_UI *p_ui, int dType, void *p_respondData)
{
    if(p_ui==NULL)
        return -1;
    switch(p_ui->uiType)
    {
    case LOGINDLG:
    {
        UserLoginDlg *loginDlg =(UserLoginDlg *)p_ui->pUi;
        loginDlg->processCallBackData(p_ui->errorCode, dType, p_respondData);
        break;
    }
    case MAINWINDOW:
    {
        DesktopListDialog *dpListDlg = (DesktopListDialog *)p_ui->pUi;
        dpListDlg->processCallBackData(p_ui->errorCode, dType, p_respondData);
        break;
    }
    case PERSONALSETTING:
    {
        CPersonalSetting *psDlg = (CPersonalSetting* )p_ui->pUi;
        psDlg->processCallBackData(p_ui->errorCode, dType, p_respondData);
        break;
    }
    case REQUESTDESKTOP:
    {
       RequestDesktopDlg *pRequestDesttop = (RequestDesktopDlg* )p_ui->pUi;
        pRequestDesttop->processCallBackData(p_ui->errorCode,dType,p_respondData);
        break;
    }
    case DESKTOP_SETTING_DLG:
    {
        DesktopSettingDialog * dsDlg = (DesktopSettingDialog*)p_ui->pUi;
        dsDlg->processCallBackData(p_ui->errorCode, dType, p_respondData);
        break;
    }
    case TERMINALDESKTOPLISTDLG:
    {
        TerminaldesktopList *pTerminalDesktopListDlg = (TerminaldesktopList*)p_ui->pUi;
        pTerminalDesktopListDlg->processCallBackData(p_ui->errorCode, dType, p_respondData);
        break;
    }
    case AUTOLOGINDLG:
    {
        AutoLoginDialog *pAutoLoginDlg = (AutoLoginDialog*)p_ui->pUi;
        pAutoLoginDlg->processCallBackData(p_ui->errorCode, dType, p_respondData);
        break;
    }
    delete p_ui;
    }

    return 0;
}

void showUiErrorCodeTip(int errorCode)
{
    switch (errorCode)
    {
    case ERROR_CONNECT_FAIL:
        CMessageBox::CriticalBox(QObject::tr("Connect fail"));
        break;
    case ERROR_USER_TICKET_INVALID:
        CMessageBox::CriticalBox(QObject::tr("User does not login"));
        break;
    case ERROR_USER_REMOVED://please donnot remove the blank in the tr()string . because in chinese it sholud add error code
        CMessageBox::CriticalBox(QObject::tr("Username or password is wrong "));//"User does not exist"));
        break;
    case ERROR_APP_REMOVED:
        CMessageBox::CriticalBox(QObject::tr("Application does not exist"));
        break;
    case ERROR_PARAMS:
        CMessageBox::CriticalBox(QObject::tr("Argument was wrong"));
        break;
    case ERROR_REQUEST_URL:
        CMessageBox::CriticalBox(QObject::tr("Remote Address request fail"));
        break;
    case ERROR_CONNECT_SERVER:
        CMessageBox::CriticalBox(QObject::tr("Could not connect to remote host"));
        break;
    case ERROR_NO_AUTHORITY:
        CMessageBox::CriticalBox(QObject::tr("No authority"));
        break;
    case ERROR_OBJECT_NOT_EXIST:
        CMessageBox::CriticalBox(QObject::tr("Object not exist"));
        break;
    case ERROR_LOGIN_PASSWORD:
        CMessageBox::CriticalBox(QObject::tr("Username or password is wrong"));
        break;
    case ERROR_LOGIN_DOMAIN:
        CMessageBox::CriticalBox(QObject::tr("Domain is wrong"));
        break;
    case ERROR_LICENSE_EXPIRED:
        CMessageBox::CriticalBox(QObject::tr("Lincense expire"));
        break;
    case ERROR_LICENSE_EXCEED:
        CMessageBox::CriticalBox(QObject::tr("User connection up to limit"));
        break;
    case ERROR_ACL_FAIL:
        CMessageBox::CriticalBox(QObject::tr("User was not permitted"));
        break;
    case ERROR_ACL_FAIL_IP:
        CMessageBox::CriticalBox(QObject::tr("IP is limited"));
        break;
    case ERROR_UNKNOWN_HOST:
        CMessageBox::CriticalBox(QObject::tr("Unknown host"));
        break;
    case ERROR_PERSONALDISK_NOT_EXIST:
        CMessageBox::CriticalBox(QObject::tr("Personal disk does not exist"));
        break;
    case ERROR_SUPPORT_TOKEN_LOGIN_ONLY:
        CMessageBox::CriticalBox(QObject::tr("Only support token login"));\
        break;
    case ERROR_SUPPORT_NORMAL_LOGIN_ONLY:
        CMessageBox::CriticalBox(QObject::tr("Only support normal login"));\
        break;
    case ERRO_GET_LICENCE_INFO_FAILED:
        CMessageBox::CriticalBox(QObject::tr("Get licence info failed"));
        break;
    case ERROR_CONNECT_DOMAIN_FAILED:
        CMessageBox::CriticalBox(QObject::tr("Connect Domain failed"));
        break;
    case ERROR_PROXY_SERVER_CLIENT_ACCESS_DENIED:
        CMessageBox::CriticalBox(QObject::tr("Current server is a proxy server, client login denied"));
        break;
    case ERROR_DESKTOPPOOL_UNREACHABLE:
        CMessageBox::CriticalBox(QObject::tr("Pool can not be used"));
        break;
    case ERROR_NO_AVAILABLE_DESKTOP_IN_POOL:
        CMessageBox::CriticalBox(QObject::tr("No available desktop resource in the desktop pool"));
        break;
    case ERROR_DESKTOPPOOL_NOT_EXIST:
        CMessageBox::CriticalBox(QObject::tr("Desktop pool does not exist"));
        break;
    case ERROR_DESKTOPPOOL_STATUS_EXCEPTION:
        CMessageBox::CriticalBox(QObject::tr("Desktop pool status exception"));
        break;
    case ERROR_DESKTOP_USED_BY_OTHER:
        CMessageBox::CriticalBox(QObject::tr("Desktop is used by other user"));
        break;
    case ERROR_DESKTOP_IP_EXCEPTION:
        CMessageBox::CriticalBox(QObject::tr("Destkop IP exception"));
        break;
    case ERROR_DESKTOP_UPDATING:
        CMessageBox::CriticalBox(QObject::tr("Pool desktop is updating"));
        break;
    case ERROR_ATTACH_DESKTOPPOOLCLOSED:
        CMessageBox::CriticalBox(QObject::tr("Attach fail, desktoopool has closed"));
        break;
    case ERROR_ATTACH_DESKTOPPOOLUNREACHABLE:
        CMessageBox::CriticalBox(QObject::tr("Attach fail, desktoppool can not be reached"));
        break;
    case ERROR_ATTACH_DESKTOPPOOLCANNOTCONNECT:
        CMessageBox::CriticalBox(QObject::tr("Attach fail, desktoppool can not connect"));
        break;
    case ERROR_UNABLE_TO_ATTACH:
        CMessageBox::CriticalBox(QObject::tr("Fail to load the virtual disk"));
        break;
    case ERROR_DESKTOP_NOT_CONNECTED:
        CMessageBox::CriticalBox(QObject::tr("Not connect to desktop, please attach after connected"));
        break;
    case ERROR_NOT_SUPPORT_FAP:
        CMessageBox::CriticalBox(QObject::tr("Not support spicec protocal"));
        break;
    case ERROR_HAS_NOT_BEEN_ASSIGNED:
        CMessageBox::CriticalBox(QObject::tr("Has not been assigned a desktop"));
        break;
    case ERROR_DESKTOPPOOL_STOP:
        CMessageBox::CriticalBox(QObject::tr("Pool has stopped"));
        break;
    case ERROR_DESKTOPPOOL_POWERING:
        CMessageBox::CriticalBox(QObject::tr("Pool is powering, please try to launch later"));
        break;
    case ERROR_VCENTER_UNREACHABLE:
        CMessageBox::CriticalBox(QObject::tr("vCenter can not be reached, please try later"));
        break;
    case ERROR_CREATE_CONNECTION_FAIL:
        CMessageBox::CriticalBox(QObject::tr("Server is busy, create a connection failure"));
        break;
    case ERROR_ANOTER_PROTOCOL_ALIVE:
        CMessageBox::CriticalBox(QObject::tr("Another protocol is being used"));
        break;
    case ERROR_APPSERVER_NOT_FOUND:
        CMessageBox::CriticalBox(QObject::tr("Can not find application server"));
        break;
    case ERROR_RDP_CLOSED:
        CMessageBox::CriticalBox(QObject::tr("Rdp service is closed"));
        break;
    case ERROR_DESKTOP_NOT_LAUNCH:
        CMessageBox::CriticalBox(QObject::tr("Desktop has not been launched"));
        break;
    case ERROR_CODE_LAUNCHPROC_QUIT_210://defined in ds_launchapp.h
        CMessageBox::CriticalBox(QObject::tr("Launch fail"));
        break;
    case ERROR_CODE_LAUNCHPROC_RDP_TIME_OUT_9://defined in ds_launchapp.h
        CMessageBox::WarnBox(QObject::tr("User doesnot use the desktop for a long time. the destop has quit automatically"));
        break;
    case ERROR_DYNAMIC_PASSWORD:
    case ERROR_EMPTY_DYNAMIC_PASSWORD:
    case ERROR_PASSWORD_EXCEED:
        CMessageBox::CriticalBox(QObject::tr("Dymaic password is wrong. (error code:") + QString::number(errorCode,10) + ")");
        break;
    case ERROR_VALID_IDENTIFICATION://case ERROR_INVALID_SEQUENCE_NUMBER:  //same erro value
        CMessageBox::CriticalBox(QObject::tr("Failed to identify. (error code:") + QString::number(errorCode,10) + ")");
        break;
    case ERROR_FAILED_FIND_DESKTOP_IP:
        CMessageBox::CriticalBox(QObject::tr("Can not find the desktop's ip!"));
        break;

    case ERROR_NOT_SUPPORT_SELFSERVICE:
        CMessageBox::WarnBox(QObject::tr("Do not support self-service capabilities"));
        break;
    case ERROR_EXCEED_DESPTOPPOOL_MAXIMUMLIMIT:
        CMessageBox::WarnBox(QObject::tr("Start the desktop pool exceeds the maximum limit"));
        break;
    case ERROR_NO_YOUR_DESKTOP_IN_POOL:
        CMessageBox::WarnBox(QObject::tr("There is no desktop belongs to you in the pool, please request one from the administrator."));
        break;

    case ERROR_FORBID_BINDING_TOKEN:
    case ERROR_EMPTY_TOKEN_IMEI:
    case ERROR_INVALID_TOKEN_TYPE:
    case ERROR_INVALID_TOKEN_SEED:
    case ERROR_TOKEN_EXPIRED:
    case ERROR_TOKEN_FREEZE:
    case ERROR_TOKEN_LOST:
    case ERROR_TOKEN_LOKED:
    case ERROR_FAIL_LOAD_KEY:
    case ERROR_FAIL_LOAD_CONFIGURATION:
        CMessageBox::CriticalBox(QObject::tr("Login failed, there has some probem with the token. (error code:") + QString::number(errorCode,10) + ")");
        break;
    default:
    {
        char string[5];
        sprintf(string, "%d", errorCode);
        CMessageBox::CriticalBox(QObject::tr("Unknown Error (ErrorCode: ") + QString(string) + ")");
        break;
    }
    }
}
