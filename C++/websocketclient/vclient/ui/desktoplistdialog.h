#ifndef DESKTOPLISTDIALOG_H
#define DESKTOPLISTDIALOG_H

#include <QDialog>
#include <vector>
#include "../common/ds_settings.h"
#include "../frontend/claunchapp.h"
#include "../common/cmutexop.h"
#include "cdesktoplistitem.h"
#include "keepsessionthread.h"
#include "../config.h"
#include "../common/cconfiginfo.h"
#include "terminaldesktoplist.h"
#include "../common/cprocessop.h"
#include "../ipc/ipcitalc.h"

namespace Ui {
class DesktopListDialog;
}
const QString BACKGROUND = ":image/resource/image/skin_expand.png";
const QString SCROLLBACKGROUND = ":image/resource/image/skin_basic.png";
enum CURSORTYPE{NORMALCURSOR, RIGHTBOTTOM, RIGHT};

class SysButton;
class QVBoxLayout;
class QSpacerItem;
class CDesktopListItem;
struct CheckDeskState;
class CLoadingDialog;
class CLaunchApp;
class QSizeGrip;
class CPersonalSetting;
class DesktopSettingDialog;
class CSelectDialog;
class RequestDesktopDlg;
class TerminaldesktopList;

struct st_reAttachVDisk_Dlg_Info
{
    LAUNCH_TYPE  launchType;
    CSelectDialog* pSelDlg;
};

class DesktopListDialog : public QDialog
{
    Q_OBJECT
    
public:
    //role : 0-teacher, 1-admin, 2-student
    explicit DesktopListDialog(const std::vector<APP_LIST>& appList, const std::vector<VIRTUALDISK>& vDisks, const std::vector<APP_LIST>& appBakList,int role = 2,QWidget *parent = 0);
    ~DesktopListDialog();
    void addDesktopItem(CDesktopListItem *);
    void processErrorCode(int errorCode, int opType);
    void processCallBackData(int errorCode, int dType, void *pRespondData);
    void setLogutProgress(uint enable);
    CDesktopListItem* getDesktopItem(QString, int& vectorNum);
    CDesktopListItem* getDesktopItem(QString);
    void logoutSession(int flag = BLOCKED);

    void switchAccessSession(int flag = BLOCKED);


    static void ipcClientProcessMsg(void *ipcMsg = NULL);
    static void *launchTerminal(void *arg);
    static QMutex s_mutex;
    static DesktopListDialog *s_desktopListDlg;
    void ipc_logoff();
    void setLogout(bool bLogout) {m_isLogout = bLogout;}
    bool getIsLogout(){return m_isLogout;}

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void closeEvent(QCloseEvent *);

    void keyPressEvent(QKeyEvent *);

signals:
    void on_signal_selfService_finished(int, int, QString, int, int);
    void on_signal_logout_finished(int errorCode, int dType);

    void on_signal_switch_access_finished(int errorCode, int dType);

    void on_signal_launch_desktop_finished(int errorCode, int dType, QString, LAUNCH_DESKTOP_DATA*);
    void on_signal_disconn_app_finished(QString);
    void on_signal_getIcon(int errorCode, void* pstIconData);

    void on_signal_logoff();
    
private slots:
    void on_pushButton_settings_clicked();

    void on_pushButton_logout_clicked();

    void on_pushButton_terminalCtl();

    void on_launch_desktop(QString);
    void on_launch_italc();

    void on_start_desktop_clicked(QString);
    void on_restart_desktop_clicked(QString);
    void on_shutdown_desktop_clicked(QString);
    void on_set_desktop_clicked(QString);
    void on_disconnect_desktop_clicked(QString);
    void on_disconnect_desktop_finished(QString);
    void on_setDefaultApp(int, LAUNCH_TYPE, APP_LIST);

    void on_pushbutton_requestBtn();

    void on_selfService_finished(int, int, QString, int, int);
    void on_logout_finish(int errorCode, int dType);

    void on_switch_access_finish(int errorCode, int dType);

    void on_keepSession_faield(int errorCode, int dType);
    void on_update_resource_list(LIST_USER_RESOURCE_DATA appList,
                                 GET_USER_INFO_DATA vDisks,
                                 GET_RESOURCE_PARAMETER resParam_out);
    void on_launch_desktop_finished(int, int, QString, LAUNCH_DESKTOP_DATA*);
    void on_getIcon(int errorCode, void* pstIconData);

    void logff_slot();


    void on_comboBox_switch_activated(const QString &arg1);


protected:
    int getAppIcon(char* appUuid);
    int updateAppIconToUi(GET_ICON_DATA* pstIconData);
    void _createTableWidgetItem();
    void _adjustItems();
    void _autoConnectToDefaultApp();

    int dealAttachDisk_desktoppool(const QString uuid, CDesktopListItem *item);

public:
     std::vector<APP_LIST> m_addlist, m_deletelist;
private:
    void __clearDesktopList();
    void __removeDesktopItem(QString);
    Ui::DesktopListDialog *ui;
    int m_role;
    bool m_isChangeSize;
    bool m_isLaunchitalc;
    bool m_isLogout;
    bool m_isMove;
    bool m_leftButtonPressed;
    std::vector<APP_LIST> m_appList;
    std::vector<VIRTUALDISK> m_vDisks;
    std::vector<APP_LIST> m_appbaklist;
    std::vector<APP_LIST> m_listtemp, m_listbak;
    CLaunchApp *m_launchApp;
    CLoadingDialog *m_loadingDlg;
    CMutexOp m_mutex, m_mutex_disconnectDesktop,m_mutex_selfservice;
    CProcessOp *m_process;
    CSession *m_pSession;
    IpcItalc *m_IpcItalc;
    RefreshListThread *m_refreshThread;

    SETTING_DEFAULTAPP m_defaultApp;
    SETTINGS_VCLIENT m_vClientSettings;
    SETTINGS_VCLIENT m_settingSet_in, m_settingSet_out;
    //used to close the dlg when the DesktopListDialog is closed.
    CPersonalSetting* m_personalSettingDlg;
    RequestDesktopDlg *m_requestDesktop;
    DesktopSettingDialog *m_desktopSettingDlg;
    TerminaldesktopList *m_terminalDesktopList;
	SysButton *sysMinimize;
	SysButton *sysClose;

    QHash<QString, st_reAttachVDisk_Dlg_Info> m_hashSelectDlg;// a list of select dlg pointer
    QHash<QString, taskUUID> m_hashGetIconThreadUuid;
                    //(Qstring is the uuid of the destop which to attach vdisk)
    QPixmap m_backgroundPixmap;
    QPoint m_pressedPoint;
    QSizeGrip * m_pQsizeGrip;
    QVBoxLayout *m_scrollAreaLayout;
    QVector<CDesktopListItem *> m_desktopItemVector;

};

#endif // DESKTOPLISTDIALOG_H
