#ifndef AUTOLOGINDIALOG_H
#define AUTOLOGINDIALOG_H

#include <QDialog>
#include <QDialog>
#include "sysbutton.h"
#include "../backend/csession.h"
#include "../common/ds_settings.h"
#include "../frontend/claunchapp.h"
#include "cselectdialog.h"
#include <QMutex>
#include <QHash>
#include <QThread>
#include "keepsessionthread.h"
#include "config.h"


class MyThread :public QThread
{
    Q_OBJECT
public:
    explicit MyThread(void*arg , void *(*threadFunc)(void *));
    void run();
private:
    void *m_arg;
    void *(* m_threadFunc)(void *);
};
struct st_reAttachVDisk_Dlg_Info;

namespace Ui {
class AutoLoginDialog;
}

class AutoLoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AutoLoginDialog(QWidget *parent = 0);
    ~AutoLoginDialog();

private:
    Ui::AutoLoginDialog *ui;
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);

    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);

    void setText(QString text);
    void setPos(int x, int y, int w = 0, int h = 0);
    void setmovieStop(bool stop);
    bool getisclosedlg(){return m_closeAutoLoginDialog;}
    void setisclosedlg(bool isclose) { m_closeAutoLoginDialog = isclose;}

public:

    static void* init(void *);
    int readNlSock(int sockFd, char *bufPtr, size_t buf_size, int seqNum, int pId);
    int parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo);
    int get_gatewayip(char *gatewayip, socklen_t size);
    void processCallBackData(int, int, void *);
    void processErrorCode(int, int);
    int getUserName();
    int initGateWay();
    int initConfig();
    int getUuid(char uuid[512]);
    int callGetUserName();
    int initDomain();
    int loginSession();
    int callListUserRes();
    int callGetUserInfo();
    int catchDesktop();
    int launchDesktop();
    int startDesktop();
    int logoutSession(int flag = BLOCKED);
    static void ipcClientProcessMsg(void *ipcMsg);
    static void * waitforDesktopstateChange(void *arg);

signals:
    void autologinFinished(int, int);
    void on_signal_launch_desktop_finished(int, int, QString,LAUNCH_DESKTOP_DATA*);
    void on_signal_selfService_finished(int, int, QString, int, int);
    void on_signal_logout_finished(int errorCode, int dType);
    void failedtogetip();

 public slots:
    void on_failed_getGateway();
    void on_logout_finish(int errorCode, int dType);
    void on_autoLogin_finished(int, int );
    void  on_local_connect();
    void on_terminal_connect();
    bool on_close_AutoLoginDialog();
    void on_launch_desktop_finished(int, int, QString,LAUNCH_DESKTOP_DATA*);
    int dealAttachDisk_desktoppool(const QString uuid, APP_LIST *appInfo);
    void on_selfService_finished(int, int, QString, int, int);
    void on_keepSession_faield(int, int);

private:
    int m_domainTaskUuid;
    int m_failGetDomainCount;
    int m_listUserResCount;
    bool m_bGetDomain;
    bool m_bLoginSession;
    bool m_hasConnectRdp, m_hasConnectFap;
    bool m_canConnect;
    bool m_isConnected, m_isStarting;
    bool m_escdown, m_ctrldown , m_closeAutoLoginDialog;
    char m_gateWay[512];
    char m_userName[512];
    char m_uuid[512];

    std::vector<APP_LIST> m_stAppList, m_stAppBakList;
    std::vector<VIRTUALDISK> m_vstVirtualDisks;
    static QMutex s_mutex;
    static AutoLoginDialog *s_autoLoginDlg;
    SETTINGS_VCLIENT m_settingSet_in, m_settingSet_out, m_vClientSettings;
    SETTINGS_LOGIN m_loginSettings;
    NT_ACCOUNT_INFO m_stNtAccountInfo;
    CLaunchApp *m_launchApp;
    APP_LIST *m_launchDesktop, *m_startDesktop;
    CSession *m_pSession;
    MyThread *m_mythread;
    SysButton* sysClose;
    SysButton* sysMax;
    QHash<QString, st_reAttachVDisk_Dlg_Info> m_hashSelectDlg;
    QPixmap m_backgroundPixmap, m_bakPixmap;
    QMovie *m_movie;
    QImage m_image;
    QStringList m_domainList;
};

#endif // AUTOLOGINDIALOG_H
