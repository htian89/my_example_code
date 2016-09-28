#ifndef CSETTINGWINDOW_H
#define CSETTINGWINDOW_H

#include <QDialog>
#include <QString>
#include "../common/ds_vclient.h"
#include "../common/ds_session.h"
#include "../common/ds_settings.h"

#define IMAGE_PATH_SETTING_WINDOWS_BACKGROUND ":image/resource/image/skin_expand.png"
//#define ICON_PATH ":image/resource/image/fronware.png"

class CUpdate;
class CSettingWindow : public QDialog
{
    Q_OBJECT
public:
    explicit CSettingWindow(const SETTINGS_VCLIENT& stSettings,  QWidget *parent = 0);
    ~CSettingWindow();
    int initialize();
    int loadSettings();
    const CMutexOp* getMutex(){ return m_pMutex_getDomain;}
    int emitUpdateDomainFinishedSignal(int state, const DOMAIN_DATA stDomainData)
        { emit getDomainFinished(state, stDomainData); return 0;}
    int getSettingResult(SETTINGS_VCLIENT& sets, DOMAIN_DATA& domaindata)
    {
        sets = m_settingSet_out;
        domaindata = m_domain_data;
        return 0;
    }    
    
signals:
    int getDomainFinished(int, const DOMAIN_DATA);
    int getKey_enterPressed(int type);// type==0 set push_ok to respose type==1 set push_exe to response
    
public slots:
    int on_btnOkPressed();
    int on_btnCancelPressed();
    int on_btnApplyPressed();
    int on_btnExecutePressed();
    int on_btnCheckUpdatePressed();
    int on_UpdateThreadFinished();
    int on_updateDomainFinished(int state, const DOMAIN_DATA stDomainData);
    int on_ChangedInWidget(QString str);
    int on_ChangedInWidget(bool b);
    void on_readOutput();
    int on_getKey_enterPressed(int);

protected:
    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent (QKeyEvent *);

private slots:
    void on_comboBox_protocol_currentIndexChanged(int index);

private:
    int setWidgetEnabled(bool bEnabled);
    int dealSettings();
    int checkValidation(bool b_ShowMsgBox = false);
    int getSettings();    
    int setLaunchOnSysStart(LOUCH_APP_ON_SYS_START isStart);
    LOUCH_APP_ON_SYS_START getLaunchOnSysStart();
public:
    taskUUID m_taskUUid;
private:
    Ui::SettingWindow * m_pUi;
    TitleWidget *m_titleWidget;

    CSession* m_pSession;
    USER_INFO m_userinfo;
    CMutexOp* m_pMutex_getDomain;

    SETTINGS_VCLIENT m_settingSet_In, m_settingSet_out;//
    DOMAIN_DATA m_domain_data;
    bool m_b_isApplyClick;

    QProcess* m_process_execCmd;
    QString m_str_cmdOutput;

    CUpdate* m_pUpdateVClientThread;

    QPoint m_pressPoint;
    bool m_isMove;    
    bool m_isNetworkSetting;
    //bool m_bUpdateDomainSucceed;
};

#endif // CSETTINGWINDOW_H
