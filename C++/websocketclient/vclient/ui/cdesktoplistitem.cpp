#include "cdesktoplistitem.h"
#include "sysbutton.h"
#include "cselfservicedialog.h"
#include "cmessagebox.h"
#include "ctooltip.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QDesktopWidget>
#include <QMutex>
#include "../common/log.h"
#include "../common/ds_launchapp.h"
#include "../common/cconfiginfo.h"
#include "../backend/csession.h"

static CDesktopListItem *sLastClickedItem=NULL;     // Mark the item which was clicked last time
QMutex* CDesktopListItem::m_pmutex_GetSelectedItem = NULL;
extern CConfigInfo* g_pConfigInfo; //defined in main.cpp
static bool g_bContextMenuShowing = false; //mark the context menu is showing. true:showing  false: not showing
static bool g_bSelfServiceShowing = false; //mark the self-service dlg is showing. true:showing  false: not showing

CDesktopListItem::CDesktopListItem(QWidget *parent) :
    QWidget(parent),
    m_busyMovie(NULL),
    m_selfServiceState(0),
    m_isBusy(false),
    m_selfServiceBtnEnable(true),
    m_contextMenu(this),
    m_toolTip(NULL),
    m_selfServiceDlg(NULL)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);
    setMouseTracking(true);

    m_itemStatus = ITEM_ENABLE;
    setMaximumHeight(70);//
    m_baseWidget = new QWidget(this);
    QHBoxLayout *Layout_onBaseWidget = new QHBoxLayout(m_baseWidget);
    Layout_onBaseWidget->setContentsMargins(3, 0, 3, 0);

    m_iconWidget = new QWidget(this);
//    m_iconWidget->setStyleSheet("background-color: rgb(122,122,122)");
    m_iconWidget->setMinimumSize(QSize(64, 64));//(QSize(72, 72));
    m_iconWidget->setMaximumSize(QSize(64, 64));//(QSize(72, 72))
    m_desktopIconLabel = new QLabel(m_iconWidget);//x, y, w ,h
    m_desktopIconLabel->setGeometry(QRect(0, 0, 64, 64));//4, 4, 64, 64 m_desktopIconLabel->setGeometry(QRect(14, 11, 43, 46));
    m_linkIconLabel = new QLabel(m_iconWidget);
    m_linkIconLabel->setGeometry(QRect(50, 47, 14, 17));//53, 53, 14, 17
    m_linkIconLabel->hide();
    m_busyStatusLabel = new QLabel(m_iconWidget);
    m_busyStatusLabel->setGeometry(QRect(0, 48, 16, 16));//0, 53, 16, 16
    m_busyStatusLabel->hide();
    m_busyMovie = new QMovie(BUSYIMAGE);
    m_busyMovie->setScaledSize(QSize(16, 16));
    m_busyStatusLabel->setMovie(m_busyMovie);
    Layout_onBaseWidget->addWidget(m_iconWidget);

    m_contentWidget = new QWidget(this);
    m_contentWidget->setFixedHeight(70);
    QHBoxLayout *contentWidgetLayout = new QHBoxLayout(m_contentWidget);
    //contentWidgetLayout->setContentsMargins(0, 0, 0, 6);
    QVBoxLayout *desktopLabelLayout = new QVBoxLayout;
    //desktopLabelLayout->setContentsMargins(6,0,0,0);
    m_desktopNameLabel = new QLabel(m_contentWidget);
    m_desktopNameLabel->setMinimumSize(QSize(140, 22));
    m_desktopNameLabel->setStyleSheet("font: bold 15px");
    m_desktopDescription = new QLabel(m_contentWidget);
    m_desktopDescription->setMinimumSize(QSize(140, 22));
    desktopLabelLayout->addWidget(m_desktopNameLabel);
    desktopLabelLayout->addWidget(m_desktopDescription);
    m_controlBtn = new SysButton("icon_button_green.png", tr("Self Service"), m_contentWidget);
    contentWidgetLayout->addLayout(desktopLabelLayout);
    contentWidgetLayout->addWidget(m_controlBtn);    
    Layout_onBaseWidget->addWidget(m_contentWidget);
    m_baseWidget->setLayout(Layout_onBaseWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_baseWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
    raise();

    connect(m_controlBtn, SIGNAL(clicked()), this, SLOT(controlBtn_clicked_slot()));
    connect(m_contextMenu.m_rdpAction, SIGNAL(triggered()), this, SLOT(on_rdpAction_clicked_slot()));
    connect(m_contextMenu.m_fapAction, SIGNAL(triggered()), this, SLOT(on_fapAction_clicked_slot()));
    connect(m_contextMenu.m_terminalAction, SIGNAL(triggered()), this, SLOT(on_terminalAction_clicked_slot()));
    connect(m_contextMenu.m_desktopSetting, SIGNAL(triggered()), this,  SLOT(on_desktopSettingAction_clicked_slot()));
    connect(m_contextMenu.m_setDefaultAppAction, SIGNAL(triggered()), this, SLOT(on_setDefaultApp_clicked_slot()));
    connect(m_contextMenu.m_cancelDefaultAppAction, SIGNAL(triggered()), this, SLOT(on_setDefaultApp_clicked_slot()));
    m_data.appData=0;
    memset(&m_data.ui_status, 0, sizeof(UI_STATUS));
}

CDesktopListItem::~CDesktopListItem()
{
    if(m_busyMovie!=NULL)
        delete m_busyMovie;
    if(sLastClickedItem==this)
        setSelectedItem(NULL);//sLastClickedItem=NULL;
    if(m_toolTip)
    {
        m_toolTip->close();
        m_toolTip = NULL;
    }
    if(NULL != m_selfServiceDlg)
    {
        delete m_selfServiceDlg;
        m_selfServiceDlg = NULL;
    }
}

void CDesktopListItem::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), palette().brush(QPalette::Window));
}

void CDesktopListItem::enterEvent(QEvent *_e)
{
    setCursor(Qt::ArrowCursor);

    /*******
    if(m_itemStatus!=ITEM_UNABLE)
    {
        QPalette pal_widget = palette();
        pal_widget.setColor(QPalette::Window, QColor(228, 255, 94));
        setPalette(pal_widget);

//        QPalette pal_icoWidget = m_iconWidget->palette();
//        pal_widget.setColor(QPalette::Window, QColor(228, 255, 94));
//        m_iconWidget->setPalette(pal_icoWidget);
        m_iconWidget->setStyleSheet("background-color: rgb(64, 124, 0)");
    }

    ***********/

    if(NULL == m_pmutex_GetSelectedItem)
    {
        m_pmutex_GetSelectedItem = new QMutex();
    }
    //qDebug()<<"enterEvent g_bContextMenuShowing"<<g_bContextMenuShowing<<g_bSelfServiceShowing;
    m_pmutex_GetSelectedItem->lock();
    if(g_bContextMenuShowing || g_bSelfServiceShowing)
    {
        ;//qDebug()<<"enterEvent g_bContextMenuShowing==true";
    }
    else if(sLastClickedItem == this)
    {
        //qDebug()<<"enterEvent getSelectedItem() == this";
        setItemBorder("QWidget{border:3px solid #008800;}");
        repaint();
    }
    else
    {
        //qDebug()<<"enterEvent ==============================";;
        setItemBorder("QWidget{border:3px solid #ccff66;}");
        if(NULL != sLastClickedItem)
        {
             sLastClickedItem->setItemBorder("QWidget{border:none;}");
        }
        sLastClickedItem = NULL;
        repaint();
    }
    m_pmutex_GetSelectedItem->unlock();
    QWidget::enterEvent( _e );
}

void CDesktopListItem::leaveEvent(QEvent *_e)
{
//    setItemStatus(m_itemStatus);

    emit mouseLeftButton();

    if(NULL == m_pmutex_GetSelectedItem)
    {
        m_pmutex_GetSelectedItem = new QMutex();
    }
    m_pmutex_GetSelectedItem->lock();
    //qDebug()<<"leaveEvent g_bContextMenuShowing"<<g_bContextMenuShowing<<"   g_bSelfServiceShowing"<<g_bSelfServiceShowing;
    if(sLastClickedItem == this)
    {
        //qDebug()<<"leaveEvent getSelectedItem() == this";
        setItemBorder("QWidget{border:3px solid #008800;}");
        repaint();
    }
    else
    {
        //qDebug()<<"leaveEvent getSelectedItem() != this";
        if(!g_bContextMenuShowing && !g_bSelfServiceShowing)
        {
            //qDebug()<<"leaveEvent !g_bContextMenuShowing";
            setItemBorder("QWidget{border:none;}");
            if(NULL != sLastClickedItem)
            {
                 sLastClickedItem->setItemBorder("QWidget{border:none;}");
            }
            sLastClickedItem = NULL;
            repaint();
        }

    }
    m_pmutex_GetSelectedItem->unlock();

    QWidget::leaveEvent(_e);
}

void CDesktopListItem::wheelEvent(QWheelEvent *_we)
{
    emit mouseLeftButton();
    QWidget::wheelEvent(_we);
}
void CDesktopListItem::mouseDoubleClickEvent(QMouseEvent *_qme)
{
    if(_qme->buttons()==Qt::LeftButton)
    {
        if(m_itemStatus == ITEM_UNABLE)
            CMessageBox::CriticalBox(tr("Desktop has not power on or unable"));
        else{
            emit on_signal_launch_desktop(m_data.uuid);
        }
    }
}

void CDesktopListItem::mousePressEvent(QMouseEvent *_me)
{
    //set press status
    qDebug() << "press: " <<m_data.uuid;
    itemPressStatus();
    //Show tool tip
    if(_me->buttons()==Qt::LeftButton && (_me->x()<70 && _me->x()>0))
    {
        if(m_toolTip == NULL)
        {
            QString descr;
            fillDesktopDescription(descr);
            QPoint p = mapToGlobal( QPoint( 0, 0 ) );
            int scr = QApplication::desktop()->isVirtualDesktop() ?
                        QApplication::desktop()->screenNumber( p ) :
                        QApplication::desktop()->screenNumber( this );


            QRect screen = QApplication::desktop()->screenGeometry( scr );


            QPixmap pixmap;
            if(m_itemStatus==ITEM_UNABLE) {
                if(0 == strcmp(m_data.appData->OsType, "Windows xp")) {
                    pixmap.load(":image/resource/image/system_icon_windows_xp_grey.png");
                } else if (0 == strcmp(m_data.appData->OsType, "Windows 7")) {
                    pixmap.load(":image/resource/image/system_icon_windows7_grey.png");
                } else if (0 == strcmp(m_data.appData->OsType, "Windows Server 2003")) {
                    pixmap.load(":image/resource/image/system_icon_windows_server2003_grey.png");
                } else if (0 == strcmp(m_data.appData->OsType, "Windows Server 2008")) {
                    pixmap.load(":image/resource/image/system_icon_windows_server2008_grey.png");
                } else {
                    pixmap.load(":image/resource/image/icon_desktop_grey_normal.png");
                }
            }
            else {
                if(m_data.appData->desktopType == TERMINAL) {
                    pixmap.load(":image/resource/image/system_icon_monitor.png");
                } else {
                    if(0 == strcmp(m_data.appData->OsType, "Windows xp")) {
                        pixmap.load(":image/resource/image/system_icon_windows_xp.png");
                    } else if (0 == strcmp(m_data.appData->OsType, "Windows 7")) {
                        pixmap.load(":image/resource/image/system_icon_windows7.png");
                    } else if (0 == strcmp(m_data.appData->OsType, "Windows Server 2003")) {
                        pixmap.load(":image/resource/image/system_icon_windows_server2003.png");
                    } else if (0 == strcmp(m_data.appData->OsType, "Windows Server 2008")) {
                        pixmap.load(":image/resource/image/system_icon_windows_server2008.png");
                    } else {
                        pixmap.load(":image/resource/image/icon_desktop_green_normal.png");
                    }
                }
            }
            m_toolTip = new CToolTip( pixmap, tr("Desktop Information"),
                                           descr,
                                           QApplication::desktop()->screen( scr ), this );
            connect( this, SIGNAL( mouseLeftButton() ),
                     this, SLOT( on_close_toolTip() ) );

            if( p.x() + m_toolTip->width() > screen.x() + screen.width() )
                p.rx() -= 4;// + tbt->width();
            if( p.y() + m_toolTip->height() > screen.y() + screen.height() )
                p.ry() -= 30 + m_toolTip->height();
            if( p.y() < screen.y() )
                p.setY( screen.y() );
            if( p.x() + m_toolTip->width() > screen.x() + screen.width() )
                p.setX( screen.x() + screen.width() - m_toolTip->width() );
            if( p.x() < screen.x() )
                p.setX( screen.x() );
            if( p.y() + m_toolTip->height() > screen.y() + screen.height() )
                p.setY( screen.y() + screen.height() - m_toolTip->height() );
            m_toolTip->move( p += QPoint( -4, height() ) );
            m_toolTip->show();
        }
    }
}

void CDesktopListItem::contextMenuEvent(QContextMenuEvent *event)
{
    //qDebug()<<"CDesktopListItem::contextMenuEvent entered";
    setSelectedItem(this);
    g_bContextMenuShowing = true;
    if(m_itemStatus != ITEM_UNABLE)
    {
        if(NULL != g_pConfigInfo)
        {
            SETTING_DEFAULTAPP default_desktop ;
            g_pConfigInfo->getSetting_defaultDesktop(default_desktop);
            CSession* pSession = CSession::GetInstance();
            if(QString(default_desktop.uuid)==m_data.uuid &&
                    QString(pSession->getUSER_INFO().username)==QString(default_desktop.userName) &&
                    QString(pSession->getNetwork().stPresentServer.serverAddress)==QString(default_desktop.serverIp) &&
                    default_desktop.isAutoConnect)
            {
                m_contextMenu.setIsDefaultApp(IS_DEFAULT_APP);
            }
            else
            {
                m_contextMenu.setIsDefaultApp(NOT_DEFAULT_APP);
                m_contextMenu.setAppSupportProtocol(m_data.ui_status.support_protocal);
            }
        }
        m_contextMenu.exec(QCursor::pos());
//        m_contextMenu.exec(this->mapToGlobal(event->pos()));
    }
    g_bContextMenuShowing = false;
    setItemBorder("QWidget{border:none;}");
    //qDebug()<<"CDesktopListItem::contextMenuEvent leaved||||||||||||||||||";
}

void CDesktopListItem::itemPressStatus()
{
    //qDebug()<<"CDesktopListItem::itemPressStatus() called....";
    //Set the item to a clicked status, and save in sLastClickedItem, to recover when user click other item
    if(m_itemStatus>=ITEM_ENABLE)//(m_itemStatus>=ITEM_ENABLE && sLastClickedItem!=this)
    {
        setItemBorder("QWidget{border:3px solid #008800;}");
        if(sLastClickedItem==this)
            return;
        //Recover the last item original status
        if(sLastClickedItem!=NULL)
        {
            //sLastClickedItem->setItemBorder("QWidget{border:none;}");
            sLastClickedItem->setDesktopIconWidget();
            sLastClickedItem->setBaseWidget();
            sLastClickedItem->setContentWidget();
            sLastClickedItem->getData().ui_status.bItemClicked = false;
        }
        setSelectedItem(this);//sLastClickedItem = this;
        m_data.ui_status.bItemClicked = true;        

//        QPalette pal_widget = palette();
//        pal_widget.setColor(QPalette::Window, QColor(0xc8, 0xfc, 0x77));//QColor(228, 255, 94));//0x8b, 0xc6, 0
//        setPalette(pal_widget);
//        //m_iconWidget->setStyleSheet("background-color: rgb(0xc8, 0xfc, 0x77)");//#9bd600 64, 124, 0 0x8b, 0xc6, 0
    }
}

void CDesktopListItem::setItemBorder(const QString strBorderStyle)
{
    m_baseWidget->setStyleSheet(strBorderStyle);//"QWidget{border:3px solid black;}"

    m_iconWidget->setStyleSheet("QWidget{border:none};");
    m_desktopIconLabel->setStyleSheet("QWidget{border:none};");
    m_busyStatusLabel->setStyleSheet("QWidget{border:none};");
    m_desktopIconLabel->setStyleSheet("QWidget{border:none};");

    m_contentWidget->setStyleSheet("QWidget{border:none};");
    m_desktopNameLabel->setStyleSheet("QWidget{border:none;font: bold 15px;};");
    m_desktopDescription->setStyleSheet("QWidget{border:none};");
}

void CDesktopListItem::fillDesktopDescription(QString &descr)
{
    const QString c_appType[5] = {tr("Application"), tr("Normal desktop"), tr(" Desktop pool"), tr("vCenter desktop"), tr("Terminal desktop")};
    const QString c_destoptoolType[2] = {tr("Auto "), tr("Manual ")};
    const QString c_appSource[2] = {tr("vCenter"), tr("Other")};
    const QString c_appAssign[] = {tr("Unknown"), tr("Special"), tr("Float"), tr("Multiple templates")};
    const QString c_powerState[3] = {tr("Unknown"), tr("Close"), tr("Open")};
    const QString c_State[3] = {tr("Unknown"), tr("Disconnected"), tr("Connected")};
    const QString c_powerNum = tr("Unknown");

    QString appType = tr("App type: ");
    QString appSource = tr("App Source: ");
    QString appAssign= tr("App assignment: ");
    QString vmState = tr("Vm state: ");
    QString rdpState = tr("Rdp state:");
    QString surplusPower = tr("Surplus power: ");
    QString surPlusRdp = tr("Surplus rdp: ");
    QString TerminalIP = tr("IP: ");
    QString TerminalState = tr("State: ");
    QString otherDescr = tr("Other descr: ");
    QString OsType = tr("OS type: ");

    APP_LIST *appData = m_data.appData;
    if(appData!=NULL)
    {
        if(appData->desktopType==DESKTOPPOOL)
        {
            if(0==appData->type || 1==appData->type)
                appType = appType + c_destoptoolType[appData->type];
            if(appData->userAssignment==0)
                appType = appType + c_appAssign[1];
            else if (appData->userAssignment == 1)
                appType = appType + c_appAssign[2];
            else if (appData->userAssignment == 2)
                appType = appType + c_appAssign[3];
            appType.append(c_appType[2] + "\n");

            if (strcmp("unknown", appData->OsType) == 0) {
                OsType = OsType + tr("unknown") + '\n';
            } else {
                OsType = OsType + appData->OsType + '\n';
            }
            if(appData->sourceType == 0)
            {
                appSource.append(c_appSource[0]+"\n");
                appAssign = "";

//                if(appData->userAssignment==0)
//                {
//                    appAssign.append(c_appAssign[1]+ "\n");
//                }
//                else
//                {
//                    appAssign.append(c_appAssign[2] + "\n");
////                    surPlusRdp.append(c_powerNum+"\n");
////                    surplusPower.append(c_powerNum+"\n");
////                    vmState.append(c_powerState[0]+"\n");
////                    rdpState.append(c_powerState[0]+"\n");
//                }

                qDebug() << appData->powerOnVmNum;
                if(appData->powerOnVmNum<0 || appData->rdpOnVmNum<0)
                {
                    surPlusRdp.append(c_powerNum+"\n");
                    surplusPower.append(c_powerNum+"\n");
                    if(appData->vmState>0)
                        vmState.append(c_powerState[2]+"\n");
                    else
                        vmState.append(c_powerState[1]+"\n");
                    if(appData->rdpServiceState>0)
                        rdpState.append(c_powerState[2]+"\n");
                    else
                        rdpState.append(c_powerState[1]+"\n");
                }
                else
                {
                    vmState.append(c_powerState[0]+"\n");
                    rdpState.append(c_powerState[0]+"\n");
                    surPlusRdp.append(QString::number(appData->rdpOnVmNum)+"\n");
                    surplusPower.append(QString::number(appData->powerOnVmNum)+"\n");
                }
            }
            else
            {
                appSource.append(c_appSource[1]+"\n");
                appAssign = "";//appAssign.append(c_appAssign[0]+ "\n");
                surPlusRdp.append(c_powerNum+"\n");
                surplusPower.append(c_powerNum+"\n");
                vmState.append(c_powerState[0]+"\n");
                rdpState.append(c_powerState[0]+"\n");
            }
        }

        else if(appData->desktopType == REMOTEDESKTOP)
        {
            appType.append(c_appType[3] + "\n");
            appSource.append(c_appSource[0] + "\n");
            appAssign="";//appAssign.append(c_appAssign[0]+"\n");
            if(appData->vmState>0)
                vmState.append(c_powerState[2]+"\n");
            else
                vmState.append(c_powerState[1]+"\n");
            if(appData->rdpServiceState>0)
                rdpState.append(c_powerState[2]+"\n");
            else
                rdpState.append(c_powerState[1]+"\n");
            surPlusRdp= "";//surPlusRdp.append(c_powerNum+"\n");
            surplusPower = "";//surplusPower.append(c_powerNum+"\n");
            if (strcmp("unknown", appData->OsType) == 0) {
                OsType = OsType + tr("unknown") + '\n';
            } else {
                OsType = OsType + appData->OsType + '\n';
            }
        }

        else if(appData->desktopType == TERMINAL) {
            appType.append(c_appType[4] + "\n");
            if(appData->state > 0)
                TerminalState.append(c_State[2]+"\n");
            else
                TerminalState.append(c_State[1]+"\n");
         }
        else
        {
            if(appData->desktopType==VIRTUALAPP)
                appType.append(c_appType[0]+"\n");
            else
                appType.append(c_appType[1]+"\n");
            appSource = "";//appSource.append(c_appSource[1] + "\n");
            appAssign = "";//appAssign.append(c_appAssign[0]+"\n");
            surPlusRdp = "";//surPlusRdp.append(c_powerNum+"\n");
            surplusPower = "";//surplusPower.append(c_powerNum+"\n");
            vmState = "";//vmState.append(c_powerState[0]+"\n");
            rdpState = "";//rdpState.append(c_powerState[0]+"\n");
        }

        if (appData->desktopType != TERMINAL) {
            otherDescr.append(QString::fromUtf8(appData->description));

            descr.append(appType);
            descr.append(appSource);
            descr.append(appAssign);
            descr.append(vmState);
            descr.append(rdpState);
            descr.append(surplusPower);
            descr.append(surPlusRdp);

            if (appData->desktopType == DESKTOPPOOL || appData->desktopType == REMOTEDESKTOP) {
                descr.append(OsType);
            }
            descr.append(otherDescr);
        } else if (appData->desktopType == TERMINAL) {
            descr.append(appType);
            descr.append(TerminalState);
        }
    }
}

void CDesktopListItem::on_close_toolTip()
{
    if(m_toolTip)
    {
        m_toolTip->close();
        m_toolTip = NULL;
    }
}

void CDesktopListItem::on_rotate_controlBtn_pixmap()
{
    m_controlBtn->setPixmap("icon_button_green.png");
}

void CDesktopListItem::on_selfServiceDlg_close()
{
    g_bSelfServiceShowing = false;
}

void CDesktopListItem::controlBtn_clicked_slot()
{
    m_controlBtn->setPixmap("icon_button_green_open.png");

    SELF_SERVICE_STATUS serviceStatus = (SELF_SERVICE_STATUS)m_selfServiceState;
    if(serviceStatus == ALLOWCLOSE)
    {
        if(m_data.ui_status.isConnected)
            serviceStatus = ALLOW_DISCONN;
        else
            serviceStatus = CANNOT_DISCONN;
    }
    setSelectedItem(this);
    g_bSelfServiceShowing = true;
    if(NULL == m_selfServiceDlg)
    {
        m_selfServiceDlg = new CSelfServiceDialog(serviceStatus, this);
        //connect(m_selfServiceDlg, SIGNAL(on_signal_start()), this, SLOT(on_desktop_start_slot()));
        connect(m_selfServiceDlg, SIGNAL(on_signal_start()), this, SLOT(on_desktop_disconnect_slot()));
        connect(m_selfServiceDlg, SIGNAL(on_signal_restart()), this, SLOT(on_desktop_restart_slot()));
        connect(m_selfServiceDlg, SIGNAL(on_signal_shutdown(int)), this, SLOT(on_desktop_shutdown_slot(int)));
        connect(m_selfServiceDlg, SIGNAL(on_signal_rotatePixmap()), this, SLOT(on_rotate_controlBtn_pixmap()));
        connect(m_selfServiceDlg, SIGNAL(on_signal_rotatePixmap()), this, SLOT(on_selfServiceDlg_close()));
    //    m_selfServiceDlg->move(QCursor::pos().x()-m_selfServiceDlg->width()+30, QCursor::pos().y()-10);
        m_selfServiceDlg->move(mapToGlobal(m_controlBtn->pos()).x()-m_selfServiceDlg->width()+90, mapToGlobal(m_controlBtn->pos()).y()+m_controlBtn->height()+5);
        m_selfServiceDlg->show();//-120+m_controlBtn->width()/2
    }
    else
    {
        m_selfServiceDlg->move(mapToGlobal(m_controlBtn->pos()).x()-m_selfServiceDlg->width()+90, mapToGlobal(m_controlBtn->pos()).y()+m_controlBtn->height()+5);
        m_selfServiceDlg->setSelfServiceStatus(serviceStatus);
        m_selfServiceDlg->show();
    }
}

//void CDesktopListItem::on_desktop_start_slot()
//{
//    emit on_signal_start_desktop(m_data.uuid);
//}

void CDesktopListItem::on_desktop_disconnect_slot()
{
    emit on_signal_disconnect(m_data.uuid);
}

void CDesktopListItem::on_desktop_restart_slot()
{
    emit on_signal_restart_desktop(m_data.uuid);
}

void CDesktopListItem::on_desktop_shutdown_slot(int iIsStart)
{//iIsStart 0 :shutdown 1 start
    LOG_INFO("iIsStart:%d", iIsStart);
    if(0 == iIsStart)
        emit on_signal_shutdown_desktop(m_data.uuid);
    else
        emit on_signal_start_desktop(m_data.uuid);
}

void CDesktopListItem::on_rdpAction_clicked_slot()
{
    //qDebug( "rdp connect");
    m_data.ui_status.connectType = RDP_CONNECT;
    emit on_signal_launch_desktop(m_data.uuid);
}

void CDesktopListItem::on_fapAction_clicked_slot()
{
    //qDebug("fap connect");
    m_data.ui_status.connectType = FAP_CONNECT;
    emit on_signal_launch_desktop(m_data.uuid);
}

void CDesktopListItem::on_terminalAction_clicked_slot()
{
    //qDebug("fap connect");
    m_data.ui_status.connectType = TERMINAL_CONNECT;
    emit on_signal_launch_desktop(m_data.uuid);
}

void CDesktopListItem::on_desktopSettingAction_clicked_slot()
{
    //qDebug("Desktop settting");
    emit on_signal_set_desktop(m_data.uuid);
}

void CDesktopListItem::on_setDefaultApp_clicked_slot()
{
    m_contextMenu.hide();
    qDebug()<<"on_setDefaultApp_clicked_slot==========";
    int iRet = 0;
    if(IS_DEFAULT_APP == m_contextMenu.isDefaultApp())
    {//currently the app is default app and the context menu is cancel default app
        APP_LIST appList = *m_data.appData;
        LAUNCH_TYPE launchType;
        emit on_signal_setDefaultApp(2, launchType, appList);
    }
    else
    {//currently the app isn't default app and the context menu is set default app
        LAUNCH_TYPE launchType;
        bool bIsOkClicked = true;
        iRet = m_contextMenu.getSetDefaultAppInfo(launchType, bIsOkClicked);
        LOG_INFO("getSetDefaultAppInfo:return value:%d, launchtype:%d, isClickedOk:%d",iRet, (int)launchType, (int)bIsOkClicked);

        if(false == bIsOkClicked)//user canceled it's settings, we don't need to do anything
            ;
        else if(iRet < 0)//get userInfo failed.
        {
            APP_LIST appList;
            emit on_signal_setDefaultApp(-1, launchType, appList);//pop up a dlg to tell the user
        }
        else//ok btn clicked. need to set it as default desktop
        {
            APP_LIST appList = *m_data.appData;
            emit on_signal_setDefaultApp(1, launchType, appList);
        }
    }
}

int CDesktopListItem::setAppIconData(char* pIconData, int len)
{
    if(NULL==pIconData || len<=0)
    {
        LOG_ERR("%s", "NULL == 0 || len<=0");
        return -1;
    }
    QPixmap pixMap;
    bool bRet = pixMap.loadFromData((uchar*)pIconData, len, "PNG");
    if(!bRet)
    {
        LOG_ERR("%s", "pixMap.loadFromData failed.");
        m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/icon_desktop_grey_normal.png").scaled(QSize(64,64),
                                                                                                           Qt::IgnoreAspectRatio,
                                                                                                           Qt::SmoothTransformation));
    }
    else
    {
        m_desktopIconLabel->setPixmap(pixMap.scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
//        m_desktopIconLabel->setPixmap(pixMap.scaled(QSize(pixMap.width(), pixMap.height()), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        getData().ui_status.bHasGetLoadIconFromVaccess = true;
    }

    return 0;

}

void CDesktopListItem::setBaseWidget()
{
    switch(m_itemStatus)
    {
    case ITEM_UNABLE:
    case ITEM_ENABLE:
    {
        QPalette pal_widget = palette();
        pal_widget.setColor(QPalette::Window, QColor(0xfa, 0xfa, 0xfa));//QColor(228, 255, 94));//0x8b, 0xc6, 0
        setPalette(pal_widget);
        break;
    }
    case ITEM_CONNECTED:
    {
        QPalette pal_widget = palette();
        pal_widget.setColor(QPalette::Window, QColor(0xcc, 0xff, 0x66));//QColor(228, 255, 94));//0x8b, 0xc6, 0
        setPalette(pal_widget);
        break;
    }
    default:
        break;
    }
}

void CDesktopListItem::setDesktopIconWidget()
{

    if(getData().appData->desktopType == VIRTUALAPP)
    {
        switch(m_itemStatus)
        {
        case ITEM_UNABLE:
            m_iconWidget->setStyleSheet("background-color: rgb(0xfa, 0xfa, 0xfa)");//rgb(122,122,122)");//186,186,186 0xcc, 0xcc, 0xcc
            if(!getData().ui_status.bHasGetLoadIconFromVaccess)
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/icon_desktop_grey_normal.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio,  Qt::SmoothTransformation));
            break;
        case ITEM_ENABLE:
            m_iconWidget->setStyleSheet("background-color: rgb(0xfa, 0xfa, 0xfa)");//rgb(132,132,132)");//216,216,216 0xcc, 0xcc, 0xcc
            if(!getData().ui_status.bHasGetLoadIconFromVaccess)
            {
               m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/icon_desktop_green_normal.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio,  Qt::SmoothTransformation));
            }
            break;
        case ITEM_CONNECTED:
            m_iconWidget->setStyleSheet("background-color: rgb(0xcc, 0xff, 0x66)");//rgb(132,132,132)");//216,216,216 0xcc, 0xcc, 0xcc

            m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/icon_desktop_green_normal.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

            break;
        default:
            break;
        }
    }
    else if (getData().appData->desktopType == REMOTEDESKTOP || getData().appData->desktopType == DESKTOPPOOL) {
        switch(m_itemStatus)
        {
        case ITEM_UNABLE:

            m_iconWidget->setStyleSheet("background-color: rgb(0xfa, 0xfa, 0xfa)");//rgb(122,122,122)");//0xcc, 0xcc, 0xcc
            if (0 == strcmp(getData().appData->OsType, "Windows xp")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows_xp_grey.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else if(0 == strcmp(getData().appData->OsType, "Windows 7")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows7_grey.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else if(0 == strcmp(getData().appData->OsType, "Windows Server 2003")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows_server2003_grey.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else if(0 == strcmp(getData().appData->OsType, "Windows Server 2008")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows_server2008_grey.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/icon_desktop_grey_normal.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            }
            break;
        case ITEM_ENABLE:

            m_iconWidget->setStyleSheet("background-color: rgb(0xfa, 0xfa, 0xfa)");//");0xcc, 0xcc, 0xcc
            if (0 == strcmp(getData().appData->OsType, "Windows xp")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows_xp.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else if(0 == strcmp(getData().appData->OsType, "Windows 7")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows7.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else if(0 == strcmp(getData().appData->OsType, "Windows Server 2003")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows_server2003.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else if(0 == strcmp(getData().appData->OsType, "Windows Server 2008")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows_server2008.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/icon_desktop_green_normal.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            }
            break;
        case ITEM_CONNECTED:

            m_iconWidget->setStyleSheet("background-color: rgb(0xcc, 0xff, 0x66)");//rgb(132,132,132)");//0xcc, 0xcc, 0xcc
            if (0 == strcmp(getData().appData->OsType, "Windows xp")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows_xp.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else if(0 == strcmp(getData().appData->OsType, "Windows 7")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows7.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else if(0 == strcmp(getData().appData->OsType, "Windows Server 2003")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows_server2003.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else if(0 == strcmp(getData().appData->OsType, "Windows Server 2008")) {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_windows_server2008.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            } else {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/icon_desktop_green_normal.png").scaled(QSize(64,64), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            }
            break;
        default:
            break;
        }
    }
    else
    {
        switch(m_itemStatus)
        {
        case ITEM_UNABLE:
    //        pal_widget.setColor(m_iconWidget->backgroundRole(), QColor(122, 122, 122));
            m_iconWidget->setStyleSheet("background-color: rgb(0xfa, 0xfa, 0xfa)");//rgb(122,122,122)");//0xcc, 0xcc, 0xcc
            m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/icon_desktop_grey_normal.png").scaled(QSize(64,64),
                                                                                                               Qt::IgnoreAspectRatio,
                                                                                                               Qt::SmoothTransformation));
            break;
        case ITEM_ENABLE:
    //        pal_widget.setColor(m_iconWidget->backgroundRole(), QColor(132, 132, 132));
            m_iconWidget->setStyleSheet("background-color: rgb(0xfa, 0xfa, 0xfa)");//");0xcc, 0xcc, 0xcc
            if(m_data.appData->desktopType == TERMINAL)
            {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_monitor.png").scaled(QSize(64,64),
                                                                                                                Qt::IgnoreAspectRatio,
                                                                                                                Qt::SmoothTransformation));
            }
            else
            {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/icon_desktop_green_normal.png").scaled(QSize(64,64),
                                                                                                            Qt::IgnoreAspectRatio,
                                                                                                          Qt::SmoothTransformation));
            }
            break;
        case ITEM_CONNECTED:
    //        pal_widget.setColor(m_iconWidget->backgroundRole(), QColor(132, 132, 132));
            m_iconWidget->setStyleSheet("background-color: rgb(0xcc, 0xff, 0x66)");//rgb(132,132,132)");//0xcc, 0xcc, 0xcc
            if(m_data.appData->desktopType == TERMINAL)
            {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/system_icon_monitor.png").scaled(QSize(64,64),
                                                                                                                Qt::IgnoreAspectRatio,
                                                                                                                Qt::SmoothTransformation));
            }
            else {
                m_desktopIconLabel->setPixmap(QPixmap(":image/resource/image/icon_desktop_green_normal.png").scaled(QSize(64,64),
                                                                                                            Qt::IgnoreAspectRatio,
                                                                                                          Qt::SmoothTransformation));
            }
            break;
        default:
            break;
        }
    }
    setDesktopIconLink();

}

void CDesktopListItem::setDesktopIconLink()
{
    switch(m_itemStatus)
    {
    case ITEM_UNABLE:
//        m_linkIconLabel->setPixmap(QPixmap(":image/resource/image/icon_button_link_unable.png").scaled(QSize(16,16),
//                                                                                                       Qt::IgnoreAspectRatio,
//                                                                                                       Qt::SmoothTransformation));
        m_linkIconLabel->setPixmap(QPixmap(""));
        m_linkIconLabel->hide();
        break;
    case ITEM_ENABLE:
//        m_linkIconLabel->setPixmap(QPixmap(":image/resource/image/icon_button_link_unable.png").scaled(QSize(16,16),
//                                                                                                       Qt::IgnoreAspectRatio,
//                                                                                                       Qt::SmoothTransformation));
        m_linkIconLabel->setPixmap(QPixmap(""));
        m_linkIconLabel->hide();
        break;
    case ITEM_CONNECTED:
        if(m_data.ui_status.connectType==0)
            m_linkIconLabel->setPixmap(QPixmap(":image/resource/image/icon_protocol_rdp.png").scaled(QSize(14,17),
                                                                                                    Qt::KeepAspectRatio,
                                                                                                    Qt::SmoothTransformation));
        else
            m_linkIconLabel->setPixmap(QPixmap(":image/resource/image/icon_protocol_fap.png").scaled(QSize(14,17),
                                                                                                    Qt::KeepAspectRatio,
                                                                                                    Qt::SmoothTransformation));
        m_linkIconLabel->show();
        break;
    default:
        break;
    }
}

void CDesktopListItem::setContentWidget()
{
    QPalette pal_widget = palette();
    switch(m_itemStatus)
    {
    case ITEM_UNABLE:
//        setStyleSheet("background-color: rgba(204, 204, 204,100%)");
        pal_widget.setColor(QPalette::Window, QColor(0xfa, 0xfa, 0xfa));//QColor(204, 204, 204));//186,186,186 0xcc, 0xcc, 0xcc
//        m_controlBtn->setPixmap("icon_button_grey.png");
        break;
    case ITEM_ENABLE:
//      setStyleSheet("background-color: rgba(234, 234, 234,100%)");
        pal_widget.setColor(QPalette::Window, QColor(0xfa, 0xfa, 0xfa));//QColor(234, 234, 234));//216,216,216 0xcc, 0xcc, 0xcc
//        m_controlBtn->setPixmap("icon_button_green.png");
        break;
    case ITEM_CONNECTED:
//        setStyleSheet("background-color: rgba(234, 234, 234,100%)");
        pal_widget.setColor(QPalette::Window, QColor(0xcc, 0xff, 0x66));//QColor(234, 234, 234));//216,216,216 0xcc, 0xcc, 0xcc
//        m_controlBtn->setPixmap("icon_button_green.png");
        break;
    default:
        break;
    }
//    if(m_selfServiceBtnEnable)
//        m_controlBtn->setPixmap("icon_button_green.png");
//    else
//        m_controlBtn->setPixmap("icon_button_grey.png");
    setPalette(pal_widget);
}

void CDesktopListItem::setItemStatus(ITEMSTATUS status)
{
    if(status == ITEM_CONNECTED) {
        m_data.ui_status.isConnected = true;
	 }
    else {
        m_data.ui_status.isConnected = false;
    }

    m_itemStatus = status;
    m_data.ui_status.item_status = m_itemStatus;

    //  If current item is clicked, check the status, when status=ITEM_UNABLE or ITEM_ENABLE, update the item status,
    //otherwise do nothing except update the link icon when status is ITEM_CONNECT
    if(sLastClickedItem==this && status==ITEM_UNABLE)
    {
        LOG_INFO("sLastClickedItem==this && status==ITEM_UNABLE");
        setSelectedItem(NULL);//sLastClickedItem = NULL;
        m_data.ui_status.bItemClicked = false;
    }

    if(status >= ITEM_ENABLE && m_data.ui_status.bItemClicked)
    {
        LOG_INFO("status >= ITEM_ENABLE && m_data.ui_status.bItemClicked");
        setBaseWidget();
        setDesktopIconWidget();
//        setDesktopIconLink();
        repaint();
        return;
    }
    LOG_INFO("setitemStatus::status:%d", status);
    setBaseWidget();
    setContentWidget();
    setDesktopIconWidget();
    repaint();
}

void CDesktopListItem::setContextMenuStatus(SUPPORT_PROTOCAL protocal)
{
    m_data.ui_status.support_protocal = protocal;
    switch(protocal)
    {
    case NO_SUPPORT:
        m_contextMenu.setRdpEnable(false);
        m_contextMenu.setFapEnable(false);
        m_contextMenu.setTerminalEnable(false);
        break;
    case BOTH_SUPPORT:
        m_contextMenu.setRdpEnable(true);
        m_contextMenu.setFapEnable(true);
        m_contextMenu.setTerminalEnable(false);
        if(m_data.appData->desktopType==DESKTOPPOOL)
        {
                if(m_data.appData->rdpOnVmNum==0 || m_data.appData->rdpServiceState==0)
                    m_contextMenu.setRdpEnable(false);
        }
        else if(m_data.appData->desktopType==REMOTEDESKTOP)
        {
            if(m_data.appData->rdpServiceState<=0)
                m_contextMenu.setRdpEnable(false);
        }
        break;
    case ONLY_RDP:
        m_contextMenu.setRdpEnable(true);
        m_contextMenu.setFapEnable(false);
#ifndef _WIN32
        m_contextMenu.setTerminalEnable(false);
#endif
        if(m_data.appData->desktopType==DESKTOPPOOL)
        {
                if(m_data.appData->rdpOnVmNum==0 || m_data.appData->rdpServiceState==0)
                    m_contextMenu.setRdpEnable(false);
        }
        else if(m_data.appData->desktopType==REMOTEDESKTOP)
        {
            if(m_data.appData->rdpServiceState<=0)
                m_contextMenu.setRdpEnable(false);
        }
        m_data.ui_status.connectType = RDP_CONNECT;
        break;
    case ONLY_FAP:
        m_contextMenu.setRdpEnable(false);
        m_contextMenu.setFapEnable(true);
#ifndef _WIN32
        m_contextMenu.setTerminalEnable(false);
#endif
        m_data.ui_status.connectType = FAP_CONNECT;
        break;
#ifndef _WIN32
    case ONLY_TERMINAL:
        m_contextMenu.setRdpEnable(false);
        m_contextMenu.setFapEnable(false);
        m_contextMenu.setTerminalEnable(true);
        m_data.ui_status.connectType = TERMINAL_CONNECT;
        break;
#endif
    default:
        break;
    }
}

void CDesktopListItem::setSelfServiceBtnEnable(bool enable)
{
    m_selfServiceBtnEnable = enable;
    m_controlBtn->setEnabled_ex(enable);
    m_controlBtn->setVisible(enable);
}

void CDesktopListItem::setSelfServiceStatus(int status/*=0*/ )
{
    m_selfServiceState = status;
    if(NULL != m_selfServiceDlg)
    {
        if(!m_selfServiceDlg->isHidden()) //has problem.. cannot work properly
            m_selfServiceDlg->move(mapToGlobal(m_controlBtn->pos()).x()-m_selfServiceDlg->width()+90, mapToGlobal(m_controlBtn->pos()).y()+m_controlBtn->height()+5);

        SELF_SERVICE_STATUS serviceStatus = (SELF_SERVICE_STATUS)status;
        if(ALLOWCLOSE == serviceStatus)
        {
            if(m_data.ui_status.isConnected)
                serviceStatus = ALLOW_DISCONN;
            else
                serviceStatus = CANNOT_DISCONN;
            m_selfServiceDlg->setSelfServiceStatus(serviceStatus);
        }
        else
            m_selfServiceDlg->setSelfServiceStatus(serviceStatus);
    }
}

void CDesktopListItem::hideSelfServiceDlg(bool bHide)
{//Bug #4345 because cannot move SelfService dlg propely. so we try to hidden it
    if(NULL != m_selfServiceDlg)
    {
        LOG_INFO("hideSelfServiceDlg:%d", int(bHide));
        if(bHide)
            m_selfServiceDlg->hide();
        else
        {
            setSelfServiceStatus(m_selfServiceState);
            setSelectedItem(this);
            g_bSelfServiceShowing = true;
            m_selfServiceDlg->show();
        }
        m_controlBtn->setPixmap("icon_button_green.png");
    }
}

void CDesktopListItem::setDesktopName(QString name)
{
    m_desktopNameLabel->setText(name);
    m_desktopNameLabel->fontMetrics().elidedText(m_desktopNameLabel->text(), Qt::ElideRight, m_desktopNameLabel->width(), Qt::TextShowMnemonic);
//    m_iconWidget->setToolTip(m_desktopNameLabel->text()+"\n"+m_desktopDescription->text());
}

void CDesktopListItem::setDesktopDescription(QString description)
{
    m_desktopDescription->setText(description);
    m_desktopDescription->fontMetrics().elidedText(m_desktopDescription->text(), Qt::ElideRight, m_desktopDescription->width(), Qt::TextShowMnemonic);
//    m_iconWidget->setToolTip(m_desktopNameLabel->text()+"\n"+m_desktopDescription->text());
}

void CDesktopListItem::setBusy(bool isBusy)
{
    m_isBusy = isBusy;
    if(m_isBusy)
    {
        m_busyStatusLabel->show();
        m_busyMovie->start();
    }
    else
    {
        m_busyStatusLabel->hide();
        m_busyMovie->stop();
    }
}

int CDesktopListItem::setSelectedItem(CDesktopListItem* pItem)
{
    if(NULL == m_pmutex_GetSelectedItem)
    {
        m_pmutex_GetSelectedItem = new QMutex();
    }
    m_pmutex_GetSelectedItem->lock();
    sLastClickedItem = pItem;
    m_pmutex_GetSelectedItem->unlock();
    return 0;
}

CDesktopListItem* CDesktopListItem::getSelectedItem()
{
    if(NULL == m_pmutex_GetSelectedItem)
    {
        m_pmutex_GetSelectedItem = new QMutex();
    }
    CDesktopListItem* pItem = NULL;
    m_pmutex_GetSelectedItem->lock();
    pItem = sLastClickedItem;
    m_pmutex_GetSelectedItem->unlock();
    return pItem;
}

int CDesktopListItem::getSelectItemInfo(std::string& str_resourceUuid, int* pIResType)
{
    int iRet = -1;
    if(NULL == m_pmutex_GetSelectedItem)
    {
        m_pmutex_GetSelectedItem = new QMutex();
    }
    m_pmutex_GetSelectedItem->lock();
    if(NULL == sLastClickedItem)
        LOG_ERR("%s","NULL == sLastClickedItem");
    else
    {
        APP_LIST* pList = sLastClickedItem->getData().appData;
        if(NULL == pList)
        {
            LOG_ERR("%s","NULL == pList");
        }
        else
        {
            str_resourceUuid = pList->uuid;
            if(NULL != pIResType)
                *pIResType = int(pList->desktopType);
            iRet = 0;
        }
    }
    m_pmutex_GetSelectedItem->unlock();
    return iRet;
}
