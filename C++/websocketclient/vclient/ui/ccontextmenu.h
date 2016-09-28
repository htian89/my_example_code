#ifndef CCONTEXTMENU_H
#define CCONTEXTMENU_H

#include <QMenu>
#include "../common/ds_vclient.h"
#include "csetdefaultapp.h"
enum IsSetDefaultApp { NOT_DEFAULT_APP = 0, IS_DEFAULT_APP };
typedef enum IsSetDefaultApp IS_SET_DEFAULT_APP;
class CSetDefaultApp;

class CSetDefaultAppMenu: public QMenu
{
public:
    CSetDefaultAppMenu(const QString & title, QWidget * parent=0);
protected:
    void resizeEvent(QResizeEvent *);
};

class CContextMenu : public QMenu
{
    Q_OBJECT
public:
    explicit CContextMenu(QWidget *parent = 0);
    ~CContextMenu();
    void setRdpEnable(bool);
    void setFapEnable(bool);

    void setTerminalEnable(bool);  //Terminal Desktop

    void setIsDefaultApp(const IS_SET_DEFAULT_APP& isDefaultApp);
    IS_SET_DEFAULT_APP isDefaultApp()
    {
        return m_isDefaultApp;
    }
    void setAppSupportProtocol(SUPPORT_PROTOCAL suppProtocol);

    int getSetDefaultAppInfo(LAUNCH_TYPE& launchType, bool &bIsOkClicked);

protected:
//    void paintEvent(QPaintEvent *);//cannot overload it,(or will show nothing on ui)
    void resizeEvent(QResizeEvent *);  // Simple Xp will be corver;
    void showEvent(QShowEvent *);

signals:

    
public slots:

private:
    void __init_menuAction();
public:    
    QAction *m_rdpAction;
    QAction *m_fapAction;
    QAction *m_terminalAction;
    QActionGroup *m_actionGroup;

    QAction *m_cancelDefaultAppAction;
    CSetDefaultApp* m_setDefaultAppAction;
    CSetDefaultAppMenu* m_DefaultAppMenu;//QMenu* m_DefaultAppMenu;
    IS_SET_DEFAULT_APP m_isDefaultApp;

    QAction *m_desktopSetting;



private:
    QString m_stylesheet_menu;
    
};

#endif // CCONTEXTMENU_H
