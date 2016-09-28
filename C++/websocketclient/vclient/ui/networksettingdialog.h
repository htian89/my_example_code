#ifndef NETWORKSETTINGDIALOG_H
#define NETWORKSETTINGDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QDecoration>
#include "../common/ds_settings.h"
#include "../common/ds_session.h"

#define IMAGE_PATH_SETTING_WINDOWS_BACKGROUND ":image/resource/image/skin_expand.png"
#define IMAGE_LOADING ":image/resource/image/loading.gif"

class QProcess;
class CSession;
class CMutexOp;
class QMovie;
class TitleWidget;
class CUpdate;
namespace Ui {
class NetWorkSettingDialog;
}

class NetWorkSettingDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit NetWorkSettingDialog(bool bShowLoginWindow = true, QWidget *parent = 0);
    ~NetWorkSettingDialog();
    const CMutexOp* getMutex(){ return m_pMutex_getDomain;}
    int emitUpdateDomainFinishedSignal(int state, const DOMAIN_DATA stDomainData)
        { emit getDomainFinished(state, stDomainData); return 0;}
    int getSettingResult(SETTINGS_VCLIENT& sets, DOMAIN_DATA& domaindata)
    {
        sets = m_settingSet_out;
        domaindata = m_domain_data;
        return 0;
    }
    int refreshAccessIpList(void);

public:
    taskUUID m_taskUuid;

protected:
    void closeEvent(QCloseEvent *event);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent (QKeyEvent *);
    void paintEvent(QPaintEvent *);
    void showEvent(QShowEvent *);  // Modify the QLineEdit nofocus;
signals:
    int getDomainFinished(int, const DOMAIN_DATA);
    int getKey_enterPressed(int);
private slots:
    void on_comboBox_protocol_currentIndexChanged(int index);
    void on_comboBox_protocol_2_currentIndexChanged(int index);
    //net cmd slot
    int on_pushBtn_executePressed();
    void on_readOutput();

    int on_radioBtn_updateType_changed(bool);//update type
    int on_checkBox_start_type_changed(bool);//vClient start type
    int on_pushBtn_apply_other_settings();
    int on_pushBtn_connect_clicked();
    int on_pushButton_cancel_clicked();
    int on_updateDomainFinished(int state, const DOMAIN_DATA stDomainData);
    int on_Changed_inNetworkSetting(QString);
    int on_Changed_inNetworkSetting(bool);
    int on_pushBtn_close_netCmd();
    int on_pushBtn_updateCheckClicked();
    int on_UpdateThreadFinished();
    int on_getKey_enterPressed(int);

    void on_pushButton_multiAccess_clicked();



private:
    int initialize();
    LOUCH_APP_ON_SYS_START getLaunchOnSysStart();
    int setLaunchOnSysStart(LOUCH_APP_ON_SYS_START isStart);
    int checkValidation(bool b_ShowMsgBox = false);
    int applySettings(int applyNetworkSetting, int applyAutoUpdate, int applyStartType);
    int getSettings(int getNetworkSetting, int getAutoUpdate, int getStartType);
    int getDomain();
    int setNetworkSettingUiEnsabled(bool bEnbled);
    int showLoadingImg(bool bShow);


private:
    Ui::NetWorkSettingDialog *ui;
    QPixmap m_backgroundPixmap;
    TitleWidget* m_titleWidget;

    bool m_isMove;
    bool m_isUiEnable;
    QPoint m_pressPoint;

    //QProcess* m_process_execCmd;
    QString m_str_cmdOutput;

    CSession* m_pSession;
    CMutexOp* m_pMutex_getDomain;
    DOMAIN_DATA m_domain_data;

    SETTINGS_VCLIENT m_settingSet_in, m_settingSet_out;
    QMovie* m_pMovie;

    bool m_bNetworkSettingChanged, m_bOtherSettingChanged; //used to specify whether the setting has saved to file
    bool m_bShowLoginWindow;

    CUpdate* m_pUpdateVClientThread;
};



class NetworkDecoration : public QDecoration
{
public:
    void buildSysMenu(QWidget *widget, QMenu *menu);
};

#endif // NETWORKSETTINGDIALOG_H
