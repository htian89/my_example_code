#ifndef USERLOGINDLG_H
#define USERLOGINDLG_H

#include <QtGui>
#include <vector>
#include "config.h"
#include "../common/ds_settings.h"
#include "../imageconf.h"
class TitleWidget;
class CSession;
class CLoadingDialog;

namespace Ui {
class UserLoginDlg;
}

const QString DOMAINBUSY = ":image/resource/image/busy.gif";

const QString LOGINBACKGROUND = vclient_image.fronview_login_background.c_str();


class UserLoginDlg : public QDialog
{
    Q_OBJECT
    
public:
    enum LOGINTYPE{NORMAL, TOKEN};
    explicit UserLoginDlg(bool b_getDomain = true, bool b_autoLogin = true,  QWidget *parent = 0, bool b_isConnect = false);
    ~UserLoginDlg();

    int initialize();

    void processCallBackData(int, int, void *);
    void processErrorCode(int, int);
    void setLoginProgress(uint enable, QString text);
    void setGetDomainBusy(bool bIsBusy);

protected:
    void showEvent(QShowEvent *);  // Modify for the QLineEdit nofucs;
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

signals:
    void updateDomainList(int , int, QStringList);
    void requestFailed(int, int);
    void loginFinished(int, int);

public slots:

    void on_combox_domainlist_update(int, int, QStringList);
    void on_settings();
    void on_loginFinished(int iSucceed, int opType);

private slots:
    void on_pushButton_login_clicked();


    void on_tabWidget_login_currentChanged(int index);

    void on_pushButton_about_clicked();


    void on_checkBox_rempwd_clicked(bool checked);

    void on_checkBox_autolg_clicked(bool checked);

    void on_pushButton_settings_clicked();

private:
    void __initDomainList();
    int loginSession();
    int callListUserRes();
    int callGetUserInfo();
    int callGetMonitorsInfo();

    Ui::UserLoginDlg *ui;
    QPixmap m_backgroundPixmap;
    LOGINTYPE m_eLoginType;
    TitleWidget *m_titleWidget;

    CSession *m_pSession;
    SETTINGS_VCLIENT m_vClientSettings;//Network *m_network;
    SETTINGS_LOGIN m_loginSettings;    
    //USER_INFO m_stTokenUserInfo;
    bool m_bGetDomain;
    bool m_bAutoLogin;
    bool m_bIsConnect;
    bool m_bSettingDlg;

    bool m_isMove;
    QPoint m_pressPoint;

    NT_ACCOUNT_INFO m_stNtAccountInfo;
    std::vector<APP_LIST> m_stAppList, m_stAppBakList;
    std::vector<VIRTUALDISK> m_vstVirtualDisks;
    int m_role;
    bool m_bHasGetListInfo, m_bHasGetUserInfo, m_bHasGetMonitorsInfo;

    CLoadingDialog* m_dlgLoading;

    QMovie *m_busyMovie;
    int m_domainTaskUuid;
    bool m_isGettingDomain;
};


#endif // USERLOGINDLG_H
