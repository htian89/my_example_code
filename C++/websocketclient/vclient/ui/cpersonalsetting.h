#ifndef CPERSONALSETTING_H
#define CPERSONALSETTING_H

#include <QDialog>
#include <QLineEdit>
#include <QFileSystemModel>
#include "../backend/csession.h"
#include "../common/ds_settings.h"
#include "sysbutton.h"


const QString IMAGE_PATH_SETTING_WINDOWS_BACKGROUND = ":image/resource/image/skin_expand.png";
const QString ICON_PATH = ":image/resource/image/fronware.png";

namespace Ui {
class CPersonalSetting;
}

class CLoadingDialog;
class CPersonalSetting : public QDialog
{
    Q_OBJECT
    
public:
    explicit CPersonalSetting(std::vector<VIRTUALDISK> iVDisks, QWidget *parent = 0);
    ~CPersonalSetting();
    void processErrorCode(int errorCode, int opType);
    void processCallBackData(int errorCode, int dType, void *pRespondData);
    bool checkInputAvailable();
    void processRequestFailed(int, int);
    void setLoadingDlg(bool enable);
    void callChangePassword();
    int getVClientSettings(SETTINGS_VCLIENT& stClientSettings)
    {
        stClientSettings = m_vClientSettings;
        return 0;
    }
    std::vector<VIRTUALDISK> setVDisk() { return m_vDisks; }

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

signals:
    void on_signal_modify_finished(int, int);
    void on_signal_modifyNt_finished(int,int);
    void on_signal_getUserInfo_finished(int, int, void*);
//    int on_checkBox_loadVDisks();
    
private slots:
    void on_pushBtn_ok_clicked();
    void on_pushBtn_ok1_clicked();

    void on_pushBtn_cancel_clicked();

    void on_modify_info_finished(int errorCode, int dType);
    void on_modify_Ntinfo_finished(int errorCode, int dType);
    void on_getUserInfo_finished(int errorCode, int dType, void* pRespondData);

    void on_tabWidget_currentChanged(int index);
    void on_treeview_clicked(QModelIndex);
    void on_apply_clicked();
    void on_checkbox_stateChange(int state);
//    void on_checkBox_loadVDisks_finished();

private:
    Ui::CPersonalSetting *ui;
    int m_current_tabIdx;
    int m_count;
    int m_iVDiskNum;
    bool m_isChangePswd;
    bool m_isChangeAccount;
    bool m_isMove;

    std::vector<VIRTUALDISK> m_vDisks;
    USER_INFO m_stUserInfo;
    NT_ACCOUNT_INFO m_accountInfo;
    SETTINGS_VCLIENT m_vClientSettings;
    CLoadingDialog *m_loadingDlg;
    CSession *m_pSession;
    CMutexOp* mutex_modify;
    SysButton* sysClose;
    QPixmap m_backgroundPixmap;
    QLineEdit *m_lineEditFilePath;
    QFileSystemModel *m_fileSystemModel;
    QPoint m_pressPoint;
};

#endif // CPERSONALSETTING_H
