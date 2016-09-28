#ifndef TERMINALDESKTOPLIST_H
#define TERMINALDESKTOPLIST_H

#include "../common/ds_settings.h"
#include "../frontend/claunchapp.h"
#include "../common/cmutexop.h"
#include "cdesktoplistitem.h"
#include "keepsessionthread.h"
#include "../config.h"
#include "../common/cconfiginfo.h"
#include "../common/ds_session.h"
#include "desktoplistdialog.h"
#include "cterminalitem.h"


#include <QDialog>
#include <QCheckBox>



class SysButton;
class QVBoxLayout;
class QSpacerItem;
struct CheckDeskState;
class CLoadingDialog;
class QSizeGrip;
class DesktopSettingDialog;

namespace Ui {
class TerminaldesktopList;
}

class TerminaldesktopList : public QDialog
{
    Q_OBJECT
    
public:
    explicit TerminaldesktopList(const std::vector<APP_LIST>& appList,QWidget *parent = 0);
    ~TerminaldesktopList();
    

signals:
    void on_signal_terminalDesktopCtl_finished(int, int);
private slots:
    void on_pushBtn_poweroff();
    void on_pushBtn_restart();
    void on_pushBtn_msg();
    void on_stateChanged_checkTerminal(string uuid,int state);
    void on_slot_checkAll(int state);
    void on_slot_terminalDesktopCtl_finised(int,int);

public:

    CTerminalItem* getTerminalItem(string uuid);
    CTerminalItem* getTerminalItem(string uuid, int &vectorNum);
    CTerminalItem* getCheckTerminalItem(string uuid, int &vectorNum);
    void addTerminalItem(CTerminalItem *item);
    void setLoadingDlg(bool enable);
    void processCallBackData(int errorCode, int dType, void *pRespondData);
    void processErrorCode(int errorCode, int dType);

protected:
    void paintEvent(QPaintEvent *);

    void mousePressEvent(QMouseEvent *_me);
    void mouseMoveEvent(QMouseEvent *_me);
    void mouseReleaseEvent(QMouseEvent *_me);

    void _createTableWidgetItem();
    void _adjustItems();
private:
    Ui::TerminaldesktopList *ui;

    bool m_checkAll, m_oneCheck;
    bool m_isMove;
    bool m_leftButtonPress;
    std::vector<APP_LIST> m_applist;
    CLoadingDialog *m_loadingDlg;
    CSession* m_session;
    SysButton* sysClose;
    SysButton* sysMinsize;
    TERMINAL_DESKTOP m_terminalDesktop;
    QPixmap m_pixmapBackground;
    QPoint m_pressPoint;
    QSizeGrip* m_pQsizeGrip;
    QVBoxLayout* m_scrollAreaLayout;
    QVector<CTerminalItem *> m_terminalItemVector,  m_stateCheckedItemVector;
};

#endif // TERMINALDESKTOPLIST_H
