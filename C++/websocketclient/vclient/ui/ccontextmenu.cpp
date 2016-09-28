#include <QPainter>
#include <QAction>
#include <QDebug>
#include <QBitmap>

#include "ccontextmenu.h"
#include "../config.h"
#include "../common/log.h"
#include "csetdefaultapp.h"

CSetDefaultAppMenu::CSetDefaultAppMenu(const QString & title, QWidget * parent/*=0*/):
    QMenu(title, parent)
{

}

void CSetDefaultAppMenu::resizeEvent(QResizeEvent *)//to impplement round corner effect
{
    QBitmap b(size());
    b.clear();
    QPainter paint_bitmap(&b);
    paint_bitmap.setBrush( Qt::color1 );
    paint_bitmap.setPen( Qt::color1 );
    paint_bitmap.drawRoundRect(0, 0, width()-1, height()-1, 4, 4);
    setMask(b);
}

///////////////////////////////////////////////////////
CContextMenu::CContextMenu(QWidget *parent) :
    QMenu(parent)
{
    m_rdpAction = NULL;
    m_fapAction = NULL;
    m_terminalAction = NULL;
    m_actionGroup = NULL;

    m_cancelDefaultAppAction = NULL;
    m_setDefaultAppAction = NULL;
    m_DefaultAppMenu = NULL;
    m_isDefaultApp = NOT_DEFAULT_APP;

    m_desktopSetting = NULL;
    setWindowOpacity(0.8);
//    setAttribute(Qt::WA_TranslucentBackground);
    if(parent==NULL)
        return;

//    setStyleSheet("QMenu { border:2px solid green; background-color: #010101; }"
//                "QMenu::separator { height: 1px; background: rgb(128,128,128); "
//                "margin-left: 5px; margin-right: 5px; }"
//                "QMenu::item {color: #ffffff;"
//                "padding: 2px 32px 2px 20px; "
//                "margin:3px; }"
//                "QMenu::item:selected { color: #e4ff5e; font-weight:bold; "
//                "background-color: #9edf04; "
//                "margin:3px; }"
//                "QMenu::item:disabled { color: gray;  margin:0px; "
//                "background-color: rgba(0,0,0,192); font-size:14px;"
//                "font-weight:bold; padding: 4px 32px 4px 20px; }" );
    m_stylesheet_menu = "QMenu { "
            "border:2px solid #e9e9e9;"
            "background-color: #e9e9e9;"
            "border-radius: 10px;}"
            "QMenu::item {color: #000000;"
            "padding: 2px 32px 2px 20px; "
            "margin:3px; border-radius:4px;}"
            "QMenu::item:selected { color: #fafafa; "
            "background-color: #505050; "
            "margin-top:3px; margin-bottom:3px;margin-left:0px;margin-right:0px;"
            "border-radius:0px;}"
            "QMenu::item:disabled { color: gray;  margin:0px; "
            "font-size:14px;"
            "font-weight:bold; padding: 4px 32px 4px 20px; }";
    setStyleSheet(m_stylesheet_menu);

    m_rdpAction = new QAction(tr("Rdp"), NULL);
#ifdef VERSION_VSAP
    m_fapAction = new QAction(tr("VSAP"), NULL);
#else
    m_fapAction = new QAction(tr("Fap"), NULL);
#endif
    m_terminalAction = new QAction(tr("Terminal"), NULL);
    connect(m_rdpAction, SIGNAL(toggled(bool)), m_rdpAction, SLOT(setChecked(bool)));
    connect(m_fapAction, SIGNAL(toggled(bool)), m_fapAction, SLOT(setChecked(bool)));
    connect(m_terminalAction, SIGNAL(toggled(bool)), m_terminalAction, SLOT(setChecked(bool)));
    m_actionGroup = new QActionGroup(this);
    m_actionGroup->setExclusive(true);
    m_actionGroup->addAction(m_rdpAction);
    m_actionGroup->addAction(m_fapAction);
#ifndef _WIN32
    m_actionGroup->addAction(m_terminalAction);
#endif
    m_rdpAction->setCheckable(true);
    m_fapAction->setCheckable(true);

    m_DefaultAppMenu = new CSetDefaultAppMenu(tr("Set as default desktop(application)"), this);//new QMenu(tr("Set as default desktop(application)"), this);
    m_DefaultAppMenu->setStyleSheet(m_stylesheet_menu);
    m_setDefaultAppAction = new CSetDefaultApp(m_DefaultAppMenu);
    m_DefaultAppMenu->addAction(m_setDefaultAppAction);
    m_cancelDefaultAppAction = new QAction(tr("Cancel default desktop(application) setting"), NULL);

    m_desktopSetting = new QAction(tr("Paralle Serial port map settings"), NULL);//QAction(tr("Desktop settings"), NULL);
    __init_menuAction();
}

CContextMenu::~CContextMenu()
{
    if(NULL != m_actionGroup)
        delete m_actionGroup;
    m_actionGroup = NULL;
    if(NULL != m_rdpAction)
        delete m_rdpAction;
    m_rdpAction = NULL;
    if(NULL != m_fapAction)
        delete m_fapAction;
    m_fapAction = NULL;
    if(NULL != m_terminalAction)
        delete m_terminalAction;
    m_terminalAction = NULL;
    if(NULL != m_DefaultAppMenu)
        delete m_DefaultAppMenu;
    m_DefaultAppMenu = NULL;
    if(NULL != m_setDefaultAppAction)
        delete m_DefaultAppMenu;
    m_DefaultAppMenu = NULL;
    if(NULL !=m_DefaultAppMenu)
        delete m_cancelDefaultAppAction;
    m_DefaultAppMenu = NULL;

    if(NULL != m_desktopSetting)
        delete m_desktopSetting;
    m_desktopSetting = NULL;
}

void CContextMenu::__init_menuAction()
{
    clear();
    addAction(m_rdpAction);
    addAction(m_fapAction);
#ifndef _WIN32
    addAction(m_terminalAction);
#endif
    addSeparator();
    if(IS_DEFAULT_APP == m_isDefaultApp)
        addAction(m_cancelDefaultAppAction);
    else
        addMenu(m_DefaultAppMenu);

    addSeparator();
    addAction(m_desktopSetting);
}

void CContextMenu::setRdpEnable(bool enable)
{
    m_rdpAction->setEnabled(enable);
    m_rdpAction->setVisible(enable);
    if(!enable)
        m_fapAction->setChecked(true);
//    if(m_fapAction->isEnabled() && !enable)
//        m_fapAction->setChecked(true);
}

void CContextMenu::setFapEnable(bool enable)
{
    m_fapAction->setEnabled(enable);
    m_fapAction->setVisible(enable);
    if(!enable)
        m_rdpAction->setChecked(true);
//    if(m_rdpAction->isEnabled() && !enable)
//        m_rdpAction->setChecked(true);
}
void CContextMenu::setTerminalEnable(bool enable)
{
    m_terminalAction->setEnabled(enable);
    m_terminalAction->setVisible(enable);
    if(!enable)
        m_terminalAction->setChecked(true);
}
void CContextMenu::setIsDefaultApp(const IS_SET_DEFAULT_APP& isDefaultApp)
{
    if(m_isDefaultApp != isDefaultApp)
    {
        m_isDefaultApp = isDefaultApp;
        __init_menuAction();
    }
}

int CContextMenu::getSetDefaultAppInfo(LAUNCH_TYPE& launchType, bool& bIsOkClicked)
{
    if(NULL == m_setDefaultAppAction)
    {
        LOG_ERR("%s","NULL==m_setDefaultAppAction");
        return -1;
    }
    return m_setDefaultAppAction->getSetDefaultAppInfo(launchType, bIsOkClicked);
}


void CContextMenu::setAppSupportProtocol(SUPPORT_PROTOCAL suppProtocol)
{
    if(NULL == m_setDefaultAppAction)
    {
        LOG_ERR("%s","NULL==m_setDefaultAppAction");
        return ;
    }
    m_setDefaultAppAction->setAppSupportProtocol(suppProtocol);
}

void CContextMenu::showEvent(QShowEvent *)
{
    if( this->isVisible() )
       this->repaint();
}

void CContextMenu::resizeEvent(QResizeEvent*)
{
    qDebug()<<"+++++++++++++++++CContextMenu::resizeEvent hascalled";
    QBitmap b(size());
    b.clear();
    QPainter paint_bitmap(&b);
    paint_bitmap.setBrush( Qt::color1 );
    paint_bitmap.setPen( Qt::color1 );
    paint_bitmap.drawRoundRect(0, 0, width()-1, height()-1, 4, 4);
    setMask(b);
}


