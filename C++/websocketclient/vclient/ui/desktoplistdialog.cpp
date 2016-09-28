#include "desktoplistdialog.h"
#include "ui_desktoplistdialog.h"
#include "sysbutton.h"
#include "../backend/csession.h"
#include "../common/log.h"
#include "../common/common.h"
#include "../common/errorcode.h"
#include "../common/cthread.h"
#include "../common/cprocessop.h"
#include "cmessagebox.h"
#include "ui_interact_backend.h"
#include "cloadingdialog.h"
#include "userlogindlg.h"
#include "cpersonalsetting.h"
#include "cselectdialog.h"
#include "desktopsettingdialog.h"
#include "requestdesktopdlg.h"
#include "../common/cconfiginfo.h"
#include "../common/MultiAccesses.h"
#include "../common/rh_ifcfg.h"
#include <QtGui>
#include <QSizeGrip>
#include "../ipc/ipcitalc.h"
#include <stdio.h>


//debug
#include <iostream>
using namespace std;

bool g_isUsbOccupy = false;
bool g_isFlashDiskOccupy = false;
bool g_isHardDiskOccupy = false;
bool g_isPrintOccupy = false;
bool g_isCDRomOccupy = false;
bool g_isOthersOccupy = false;
bool g_bMapFileSystemOccupy = false;
bool g_bSystemShutDownSignal = false;
extern bool vclass_flag;
extern std::string version;

//  g_desktopListDlg only used to call its destructor (delete g_desktopListDlg),
//and we do not suggest you to use it for others.
//  the reason to add this global parameter is that if we destruct it when after responset to close signal,
//it may has some dlg not disppear while the destructor has called. it may lead the vClient
//to crash -->so we destruct it when the next time before construct it.
DesktopListDialog* g_desktopListDlg = NULL;

extern CConfigInfo* g_pConfigInfo; //defined in main.cpp

DesktopListDialog::DesktopListDialog(const std::vector<APP_LIST>& appList,
                                     const std::vector<VIRTUALDISK>& vDisks, const std::vector<APP_LIST>& appBakList,int role, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DesktopListDialog),
    m_role(role),
    m_isChangeSize(false),
    m_isLogout(false),
    m_isMove(false),
    m_leftButtonPressed(false),
    m_appList(appList),
    m_vDisks(vDisks),
    m_appbaklist(appBakList),
    m_launchApp(NULL),
    m_loadingDlg(NULL),
    m_process(NULL),
    m_pSession(NULL),
    m_IpcItalc(NULL),
    m_refreshThread(NULL),
    m_personalSettingDlg(NULL),
	m_requestDesktop(NULL),
    m_desktopSettingDlg(NULL),
    m_terminalDesktopList(NULL),
    m_pQsizeGrip(NULL)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowMinimizeButtonHint);
    if(isWindow()){
        qDebug() << "this is independent window";
    }
    //setAttribute(Qt::WA_DeleteOnClose);
    //setWindowIcon(QIcon(WINDOWS_ICON));
    setWindowIcon(QIcon(WINDOWS_IMG.data()));
    setMouseTracking(true);
    m_pSession = CSession::GetInstance();
    g_isUsbOccupy = false;
    //Set background picture
    m_backgroundPixmap.load(BACKGROUND);
    ui->label_companylogo->hide();
    setDialogInCenter(this);

    // set this, the dialog window will ajust its size automatically
    //setSizeGripEnabled(true); //because it has shadow, the ui of QSizeGrop will on the shadow, no on the ui

    //Add close button and minimize button
    sysClose = new SysButton("icon_close.png", tr("Close"), this);
    sysMinimize = new SysButton("icon_minimize.png", tr("Minimize"), this);
    connect(sysClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(sysMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
    ui->horizontalLayout_top->addStretch();
    ui->horizontalLayout_top->addWidget(sysMinimize);
    ui->horizontalLayout_top->addWidget(sysClose);
    //ui->label_color->setPixmap(QPixmap(CORNER_TOP_LEFT_IMG));
    ui->label_color->setPixmap(QPixmap(vclient_image.corner_left_top.data()));

/* 2015-2-6 add*/
    if (version.find("HXXY") != std::string::npos)
    {
        setWindowTitle("Thinview");
        ui->label_productName->setText(tr("Thinview"));
    }
    else if (version.find("sugon_2000") != std::string::npos)
    {
        setWindowTitle("Fronview2000");
        ui->label_productName->setText(tr("Fronview2000"));
    }
    else if (version.find("vclass") != std::string::npos)
    {
        setWindowTitle("vClass");
        ui->label_productName->setText(tr("vClass"));
    }
    else if (version.find("JSJQ") != std::string::npos)
    {
        setWindowTitle(tr("Cloud desktop platform"));
        //ui->label_productName->setPixmap(QPixmap(PRODUCT_NAME_IMG_PATH));
        ui->label_productName->setPixmap(QPixmap(vclient_image.jsjq_title.data()));
        ui->label_companylogo->setVisible(true);
        //ui->label_companylogo->setPixmap(QPixmap(COMPANY_LOGO));
        ui->label_companylogo->setPixmap(QPixmap(vclient_image.logo_jsjq.data()));
        ui->label_companylogo->setFixedWidth(110);
    }
    else
    {
        setWindowTitle("Fronview3000");
        ui->label_productName->setText(tr("Fronview3000"));
    }

    //ui->label_vertical_line->setPixmap(QPixmap(VERTICAL_LINE_PIC));//label_vertical_line
    ui->label_vertical_line->setPixmap(QPixmap(vclient_image.vertical_line.c_str()));//label_vertical_line
    ui->label_username->setText(QString::fromUtf8(m_pSession->getUSER_INFO().username));

    m_pQsizeGrip = new QSizeGrip(this);
    m_pQsizeGrip->setStyleSheet("image: url(:image/resource/image/icon_enlarge.png);");
    m_pQsizeGrip->resize(m_pQsizeGrip->sizeHint());
    m_pQsizeGrip->move(rect().bottomRight().x()-m_pQsizeGrip->rect().bottomRight().x() -15, \
                    rect().bottomRight().y()-m_pQsizeGrip->rect().bottomRight().y() -15);
    //Set the widget's style
    QString buttonStyle = "QPushButton{"
            "color: #515151;"
            "font: bold 11pt;"
            "border-radius: 0px;"
            "selection-color:white;}"
            "QPushButton:hover{"
            "color: #101010;"
            "font: bold ;}"
            "QPushButton:pressed{"
            "color: #000000;}";//b7b7b7 bold
    ui->pushButton_logout->setStyleSheet(buttonStyle);
	ui->pushButton_logout->setFocusPolicy(Qt::NoFocus);
    ui->pushButton_settings->setStyleSheet(buttonStyle);
    ui->pushButton_settings->setFocusPolicy(Qt::NoFocus);
    ui->requestDesktopbtn->setStyleSheet(buttonStyle);
    ui->requestDesktopbtn->setFocusPolicy(Qt::NoFocus);
    ui->pushBtn_terminalCtl->setStyleSheet(buttonStyle);
    ui->pushBtn_terminalCtl->setFocusPolicy(Qt::NoFocus);
    ui->pushBtn_terminalCtl->hide();

    if(m_role == 0)
    {
        ui->pushBtn_eClass->setVisible(true);
        ui->pushBtn_eClass->setStyleSheet(buttonStyle);
        connect(ui->pushBtn_eClass, SIGNAL(clicked()), this, SLOT(on_launch_italc()));
    }else{
        ui->pushBtn_eClass->setVisible(false);
    }
	//now donot use eClass, hide it;
	ui->pushBtn_eClass->setVisible(false);
    if (vclass_flag)
        ui->requestDesktopbtn->setVisible(false);
	
    //Set scroll area background color
    QPalette scrocontentPal = ui->scrollAreaWidgetContents->palette();
    scrocontentPal.setBrush(QPalette::Window, QBrush(QColor(0xfa, 0xfa, 0xfa)));//0xe9, 0xe9, 0xe9 scrocontentPal.setBrush(QPalette::Window, QBrush(QPixmap(SCROLLBACKGROUND)));
    ui->scrollAreaWidgetContents->setPalette(scrocontentPal);

    m_scrollAreaLayout = new QVBoxLayout(ui->scrollAreaWidgetContents);
    m_scrollAreaLayout->setContentsMargins(3, 0, 6, 0);//(6, 0, 6, 0)
    m_scrollAreaLayout->setSpacing(1);//default is 6

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 145, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_scrollAreaLayout->addItem(verticalSpacer);

#ifdef _WIN32
    ui->comboBox_switch->hide();
#endif

#ifndef _WIN32
    MultiAccesses *pMultiAccesses = new MultiAccesses;
    pMultiAccesses->readFile();
    int mA_size = pMultiAccesses->size();
    while (mA_size > 0) {
        if(strlen(pMultiAccesses->top().AccessIp) > 0){
            ui->comboBox_switch->addItem(pMultiAccesses->top().AccessIp);
        }
        pMultiAccesses->pop();
        mA_size--;
    }
    pMultiAccesses->~MultiAccesses();
#endif

    //read the default app
    memset(&m_defaultApp, 0, sizeof(m_defaultApp));
    if(g_pConfigInfo!=NULL)
    {
        g_pConfigInfo->getSetting_defaultDesktop(m_defaultApp);
        int iRet = g_pConfigInfo->getSettings_vclient(m_vClientSettings);
        if(iRet < 0)
        {
            LOG_ERR("%s", "get config info from file failed");
            memset(&m_vClientSettings, 0, sizeof(SETTINGS_VCLIENT));
        }
        if(m_vClientSettings.m_fapBar == UNKNOWN_BAR_STATUS)
            m_vClientSettings.m_fapBar = HASBAR_STATE;
        if(m_vClientSettings.m_rdpBar == UNKNOWN_BAR_STATUS)
            m_vClientSettings.m_rdpBar = HASBAR_STATE;
    }

    //initialize desktop list
    _createTableWidgetItem();
    connect(this, SIGNAL(on_signal_getIcon(int,void*)), this, SLOT(on_getIcon(int,void*)));
    for(unsigned int i = 0; i<m_appList.size(); i++)
    {
        if(m_appList[i].desktopType == VIRTUALAPP)
        {
            getAppIcon(m_appList[i].uuid);
        }
    }

    //Create the keep session and refresh resource list theads
//    m_keepSessionThread = new KeepSessionThread;
    m_refreshThread = new RefreshListThread;
//    connect(m_keepSessionThread, SIGNAL(on_signal_keepSession_finished(int, int)), this, SLOT(on_keepSession_faield(int, int)));
    connect(m_refreshThread, SIGNAL(on_signal_update_failed(int, int)), this, SLOT(on_keepSession_faield(int, int)));
    connect(m_refreshThread, SIGNAL(on_signal_update_resourceList(LIST_USER_RESOURCE_DATA, GET_USER_INFO_DATA,GET_RESOURCE_PARAMETER)),
            this, SLOT(on_update_resource_list(LIST_USER_RESOURCE_DATA ,GET_USER_INFO_DATA,GET_RESOURCE_PARAMETER)));
//    m_keepSessionThread->start();
    m_refreshThread->start();

//    _initWebSocket();

    connect(this, SIGNAL(on_signal_selfService_finished(int, int, QString, int, int)),
            this, SLOT(on_selfService_finished(int, int, QString, int,int)));
    connect(this, SIGNAL(on_signal_logout_finished(int, int)), this, SLOT(on_logout_finish(int, int)));

    connect(this, SIGNAL(on_signal_switch_access_finished(int, int)), this, SLOT(on_switch_access_finish(int, int)));

    connect(this, SIGNAL(on_signal_launch_desktop_finished(int, int, QString,LAUNCH_DESKTOP_DATA*)),
                this, SLOT(on_launch_desktop_finished(int, int, QString,LAUNCH_DESKTOP_DATA*)));
    connect(this, SIGNAL(on_signal_disconn_app_finished(QString)), this, SLOT(on_disconnect_desktop_finished(QString)));
    connect(this, SIGNAL(on_signal_logoff()), this, SLOT(logff_slot()));
    connect(ui->requestDesktopbtn, SIGNAL(clicked()), this, SLOT(on_pushbutton_requestBtn()));
    connect(ui->pushBtn_terminalCtl, SIGNAL(clicked()), this, SLOT(on_pushButton_terminalCtl()));


    s_desktopListDlg = this;
    _autoConnectToDefaultApp();
}

DesktopListDialog::~DesktopListDialog()
{
    LOG_ERR("%s","~DesktopListDialog() is called");
    delete ui;
    if(m_refreshThread!=NULL)
        delete m_refreshThread;
    if(m_process != NULL)
    {
        m_process->termate();
        delete m_process;
    }
    if(m_IpcItalc != NULL)
    {
        delete m_IpcItalc;
        m_IpcItalc = NULL;
    }
    LOG_ERR("%s","~DesktopListDialog() finished....");
}

void DesktopListDialog::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setBrush(Qt::NoBrush);
//   QPen pen;
//    QColor color(0, 0, 0, 50);
//   for(int i=5; i>=4; i--)
//  {
//     color.setAlpha(50 - (i-1)*10);
//        pen.setColor(color);
//        paint.setPen(pen);
//        paint.drawRoundRect(QRectF(5+i, i, width()-11, height()-6), 2, 2);
//    }
//    for(int i=3; i>=2; i--)
//    {
//        color.setAlpha(70 - (i-1)*12);
//        pen.setColor(color);
//        paint.setPen(pen);
//        paint.drawRoundRect(QRectF(5+i, i, width()-11, height()-6), 2, 2);
//    }
//    for(int i=1; i>=0; i--)
//    {
//       color.setAlpha(100 - (i-1)*20);
//        paint.setPen(color);
//        paint.drawRoundRect(QRectF(5+i, i, width()-11, height()-6), 2, 2);
//    }
    paint.setBrush(QBrush(m_backgroundPixmap));
    paint.setPen(QPen(QColor(0xe9, 0xe9, 0xe9)));
    paint.drawRoundRect(QRectF(0,0,width(),height()),0,0);
   // paint.drawRoundRect(QRectF(2, 0, width()-8, height()-6), 2, 2);

//    QPainter paint(this);
//    paint.setBrush(QBrush(m_backgroundPixmap));
//    QRectF rectfContent(2, 0, width()-8, height()-6);
//    paint.setPen(QPen(QColor(0xe9, 0xe9, 0xe9)));
//    paint.drawRoundRect(rectfContent, 2, 2);
//    //paint.drawRoundedRect( rectfContent.translated(0.5,0.5), 2.0,2.0 );

//    QPen pen;
//    //QVector<qreal> dashes;
//    /*dashes<<1<<1;
//    pen.setDashPattern(dashes);*/
//    QColor color(0, 0, 0, 50);
//    for(int i=0; i<2; i++)
//    {
//        color.setAlpha(100 - (i-1)*20);
//        paint.setPen(color);
//        //paint.drawLine(6-i, i, 6-i, height()-(7-i));//X1, Y1, X2, Y2
//        paint.drawLine(width()-(6-i), i+3, width()-(6-i), height()-(7-i));
//        paint.drawLine(i+4, height()-(6-i), width()-(6-i), height()-(6-i));
//    }
//    //paint.drawArc(QRectF(width()-8, height()-6, width()-6, height()-4),0, 90);
////    dashes.clear();
////    dashes<<1<<5;
//    for(int i=2; i<4; i++)
//    {
//        color.setAlpha(60 - (i-1)*12);
//        pen.setColor(color);
//        paint.setPen(pen);
//        //paint.setPen(color);
//        //paint.drawLine(6-i, i, 6-i, height()-(7-i));
//        paint.drawLine(width()-(6-i), i+2, width()-(6-i), height()-(7-i));
//        paint.drawLine(i+3, height()-(6-i), width()-(7-i), height()-(6-i));
//    }
////    dashes.clear();
////    dashes<<1<<10;
//    for(int i=4; i<6; i++)
//    {
//        color.setAlpha(50 - (i-1)*10);
//        pen.setColor(color);
//        paint.setPen(pen);
//        //paint.setPen(color);
//       // paint.drawLine(6-i, i, 6-i, height()-(7-i));
//        paint.drawLine(width()-(6-i), i+2, width()-(6-i), height()-(7-i));
//        paint.drawLine(i+3, height()-(6-i), width()-(7-i), height()-(6-i));
//    }


/*    QColor color(0, 0, 0, 50);
    for(int i=0; i<2; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        color.setAlpha(80 - (i-1)*10);
        paint.setPen(color);
        //paint.drawLine(10-i, i, 10-i, height()-(11-i));
        paint.drawLine(width()-(10-i), i, width()-(10-i), height()-(11-i));
        paint.drawLine(10-i, height()-(10-i), width()-(11-i), height()-(10-i));
    }
    for(int i=2; i<5; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        color.setAlpha(40 - (i-1)*4);
        paint.setPen(color);
        paint.drawLine(10-i, i, 10-i, height()-(11-i));
        paint.drawLine(width()-(10-i), i, width()-(10-i), height()-(11-i));
        paint.drawLine(10-i, height()-(10-i), width()-(11-i), height()-(10-i));
    }
    for(int i=5; i<10; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        color.setAlpha(20 - (i-1)*2);
        paint.setPen(color);
        paint.drawLine(10-i, i, 10-i, height()-(11-i));
        paint.drawLine(width()-(10-i), i, width()-(10-i), height()-(11-i));
        paint.drawLine(10-i, height()-(10-i), width()-(11-i), height()-(10-i));
    }*/

    if(NULL != m_pQsizeGrip)
        m_pQsizeGrip->move(rect().bottomRight().x()-m_pQsizeGrip->rect().bottomRight().x() -7, \
                        rect().bottomRight().y()-m_pQsizeGrip->rect().bottomRight().y() -7);

}

void DesktopListDialog::mouseMoveEvent(QMouseEvent *_me)
{
    if( (_me->buttons() == Qt::LeftButton) && m_isMove)
    {
//        QPoint nowPoint = pos();
//        nowPoint.setX(nowPoint.x()+_me->pos().x()-pressedPoint.x());
//        nowPoint.setY(nowPoint.y()+_me->pos().y()-pressedPoint.y());
//        this->move(nowPoint);
        move(_me->globalPos() - m_pressedPoint);
    }
/*    else
    {
        CURSORTYPE cursorType = __getSideType(_me->pos());
        __setCursorType(cursorType);

        if(_me->buttons() == Qt::LeftButton && m_leftButtonPressed && m_isChangeSize)
            __ChangeSize(_me->globalPos());
    }
*/
}

void DesktopListDialog::mousePressEvent(QMouseEvent *_me)
{
    //    pressedPoint = _me->pos();
    if(_me->button() == Qt::LeftButton)
    {
        m_leftButtonPressed = true;
        if(!m_isChangeSize)
            m_isMove = true;
        m_pressedPoint = _me->globalPos()-pos();
    }
}

void DesktopListDialog::mouseReleaseEvent(QMouseEvent *)
{
    if(m_isMove)
        m_isMove = false;
    m_leftButtonPressed = false;
}

void DesktopListDialog::keyPressEvent(QKeyEvent *keyevent)
{
    switch(keyevent->key())
    {
    case Qt::Key_Escape:
        this->close();
        break;
    default:
        QDialog::keyPressEvent(keyevent);
    }
}

void DesktopListDialog::closeEvent(QCloseEvent *_e)
{
    QMutexLocker mutexLocker(&s_mutex);
    s_desktopListDlg = NULL;
    if(m_isLogout)
        return;
    LOG_ERR("%s","hasnot logout");

    //Receive the system logout , shutdown and reboot signal ,do this
    if(g_bSystemShutDownSignal)
    {
          LOG_ERR("%s", "system shut down ,close the dialog");
          logoutSession(BLOCKED);
        _e->accept();
    }
   else  if( CMessageBox::SelectedBox(tr("Are you sure to quit vClient?")) == REJECTED)
    {
        _e->ignore();
        return;
    }
    else
    {        
        if(m_process)
        {
            m_process->termate();
        }
        if(m_IpcItalc)
        {
            delete m_IpcItalc;
            m_IpcItalc = NULL;
        }
        hide();
        if(m_refreshThread)
        {
            m_refreshThread->setStop(true);
            m_refreshThread->wait();
        }
        logoutSession(BLOCKED);
        m_mutex.lock();
        for(QHash<QString, taskUUID>::iterator iter=m_hashGetIconThreadUuid.begin(); iter != m_hashGetIconThreadUuid.end();)
        {
            if(NULL != m_pSession)
            {
                m_pSession->cancelTask(iter.value());
                LOG_INFO("has erase m_hashGetIconThreadUuid, key:value:%s,%d", iter.key().toUtf8().data(), iter.value());
            }
            m_hashGetIconThreadUuid.erase(iter++);
        }
        m_mutex.unlock();
        if(NULL != m_pSession)
        {
            m_pSession = NULL;
            CSession::Release();
        }
        _e->accept();
    }
}

void DesktopListDialog::setLogutProgress(uint enable)
{
    if(m_loadingDlg==NULL)
        m_loadingDlg = new CLoadingDialog(tr("Logouting ..."), this);
    else
        m_loadingDlg->setText(tr("Logouting ..."));
    m_loadingDlg->setPos(0, 5,width(), height());
    m_loadingDlg->setMovieStop(enable^1);
    m_loadingDlg->setVisible(enable);
    ui->scrollAreaWidgetContents->setVisible(enable^1);
}

void DesktopListDialog::_autoConnectToDefaultApp()
{
    DefaultDesktop defaultDesktop;
    g_pConfigInfo->getSetting_defaultDesktop(defaultDesktop);

    /*if((getDesktopItem(QString(defaultDesktop.uuid))!=NULL) &&
             (QString(m_pSession->getNetwork().presentServer)==QString(defaultDesktop.serverIp)) &&
             (QString(m_pSession->getUSER_INFO().username)==QString(defaultDesktop.userName)) &&
            defaultDesktop.isAutoConnect)*/
    if((getDesktopItem(QString(defaultDesktop.uuid))!=NULL) &&
             (QString(m_pSession->getNetwork().stPresentServer.serverAddress)==QString(defaultDesktop.serverIp)) &&
             (QString(m_pSession->getUSER_INFO().username)==QString(defaultDesktop.userName)) &&
            defaultDesktop.isAutoConnect)
    {
        int reference, protocolSupport = -1;
        CDesktopListItem *item = getDesktopItem(defaultDesktop.uuid, reference);

        if(NULL != item->getData().appData)
            protocolSupport= item->getData().appData->displayprotocol;
        if(NULL!=item
          && (ITEM_UNABLE ==item->getData().ui_status.item_status
              ||(0!=protocolSupport && protocolSupport-1!=defaultDesktop.connectProtocal)))
        {//the desktop is disabled. or protocol deesnot support
            CMessageBox::CriticalBox((tr("The default desktop cannot be connect.\n(possible reason:the desktop doesnot poweron;no available destop;the protocol doesnot support)")));
            return;
        }
        item->getData().ui_status.connectType =(CONNECT_TYPE)(defaultDesktop.connectProtocal);
        on_launch_desktop(QString(defaultDesktop.uuid));
    }
}

int DesktopListDialog::getAppIcon(char* appUuid)
{
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return -1;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = MAINWINDOW;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();

    m_pSession->getIcon(param, appUuid);
    m_mutex.lock();
    m_hashGetIconThreadUuid.insert(appUuid, param.taskUuid);
    m_mutex.unlock();
    return 0;
}

void DesktopListDialog::on_getIcon(int errorCode, void* pstIconData)
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        return;
    }
    GET_ICON_DATA* pIconData = (GET_ICON_DATA*)pstIconData;
    if(errorCode<0)
    {
        LOG_ERR("get Icon failed. return value:%d", errorCode);
    }
    else
    {
        qDebug()<<"get ICON succeed. len:"<<pIconData->iLen<<endl;
        updateAppIconToUi(pIconData);
    }
}

int DesktopListDialog::updateAppIconToUi(GET_ICON_DATA* pstIconData)
{
    if(NULL == pstIconData)
        return -1;

    int refrence=0;
    CDesktopListItem *item;
    item = getDesktopItem(pstIconData->strAppUuid.c_str(), refrence);
    if(item!=NULL)
    {
        item->setAppIconData(pstIconData->data, pstIconData->iLen);
    }
    else
    {
        LOG_ERR("cannot find item: uuid:%s", pstIconData->strAppUuid.c_str());
    }
    if(NULL != pstIconData->data)
        free(pstIconData->data);
    return 0;
}
void DesktopListDialog::_createTableWidgetItem()
{  
    LOG_INFO("_createTableWidgetItem");

    for(std::vector<APP_LIST>::size_type i=0; i<m_appList.size(); ++i)
    {

	        ITEM_DATA data;
	        CDesktopListItem *item = getDesktopItem(QString(m_appList.at(i).uuid));
	        if(item == NULL)
	        {
	            item = new CDesktopListItem(ui->scrollAreaWidgetContents);
                connect(item, SIGNAL(on_signal_launch_desktop(QString)), this, SLOT(on_launch_desktop(QString)));
	            connect(item, SIGNAL(on_signal_start_desktop(QString)), this,SLOT(on_start_desktop_clicked(QString)));
	            connect(item, SIGNAL(on_signal_restart_desktop(QString)), this, SLOT(on_restart_desktop_clicked(QString)));
	            connect(item, SIGNAL(on_signal_shutdown_desktop(QString)), this, SLOT(on_shutdown_desktop_clicked(QString)));
	            connect(item, SIGNAL(on_signal_set_desktop(QString)), this, SLOT(on_set_desktop_clicked(QString)));
	            connect(item, SIGNAL(on_signal_disconnect(QString)), this, SLOT(on_disconnect_desktop_clicked(QString)));
	            connect(item, SIGNAL(on_signal_setDefaultApp(int,LAUNCH_TYPE,APP_LIST)), this, SLOT(on_setDefaultApp(int,LAUNCH_TYPE,APP_LIST)));
	            data.appData=0;
	            memset(&data.ui_status, 0, sizeof(data.ui_status));
	            data.appData = &m_appList[i];
	            data.uuid = QString(data.appData->uuid);
	            item->setDesktopName(QString::fromUtf8(data.appData->name));
	            item->setDesktopDescription(QString::fromUtf8(data.appData->description));
	            item->setData(data);
	            addDesktopItem(item);
	        }
	        else
	        {
	            //Do not change the data when the desktop is do self service.
	            data.appData = &m_appList[i];
	            item->getData().appData = &m_appList[i];
	            item->setDesktopName(QString::fromUtf8(data.appData->name));
	            item->setDesktopDescription(QString::fromUtf8(data.appData->description));
	            if(item->getData().ui_status.isStarting
	                    || item->getData().ui_status.isReStarting
	                    || item->getData().ui_status.isShutdown
	                    || item->getData().ui_status.isConnected)
	                continue;
	        }
	        switch(data.appData->desktopType)
	        {
	            case VIRTUALAPP:
	            case NORMALDESKTOP:
	                item->setItemStatus(ITEM_ENABLE);
	                item->setSelfServiceBtnEnable(false);
	                item->setContextMenuStatus((SUPPORT_PROTOCAL)data.appData->displayprotocol);
	                break;
	            case DESKTOPPOOL:
	            {
	                if(data.appData->sourceType==0)
	                {
	                    if(data.appData->vmState>0 || data.appData->powerOnVmNum>0 || data.appData->rdpOnVmNum>0)
	                        item->setItemStatus(ITEM_ENABLE);
	                    else
	                        item->setItemStatus(ITEM_UNABLE);
                        if(data.appData->userAssignment==0 || data.appData->userAssignment ==2)
	                    {
	                        item->setSelfServiceBtnEnable(true);
	                        if(data.appData->vmState>0 || data.appData->rdpServiceState>0)
	                            item->setSelfServiceStatus(0);
	                        else if(data.appData->powerOnVmNum>=0 || data.appData->rdpOnVmNum>=0)
	                            item->setSelfServiceStatus(-1);
	                        else
	                            item->setSelfServiceStatus(1);
	
	                    }
	                    else
	                        item->setSelfServiceBtnEnable(false);
	                }
	                else
	                {
	                    item->setItemStatus(ITEM_ENABLE);
	                    item->setSelfServiceBtnEnable(false);
	                }
	                item->setContextMenuStatus((SUPPORT_PROTOCAL)data.appData->displayprotocol);
	                break;
	            }
	            case REMOTEDESKTOP:
	            {
	                if(data.appData->vmState>0)
	                {
	                    item->setSelfServiceStatus(0);
	                    item->setItemStatus(ITEM_ENABLE);
	                }
	                else
	                {
	                    item->setSelfServiceStatus(1);
	                    item->setItemStatus(ITEM_UNABLE);
	                }
	                item->setSelfServiceBtnEnable(true);
	                item->setContextMenuStatus((SUPPORT_PROTOCAL)data.appData->displayprotocol);
	                break;
	            }
	            case TERMINAL:
	            {
                    ui->label_companylogo->hide();
                    ui->pushBtn_terminalCtl->setVisible(true);

                    //if(data.appData->state>0)
                    //{
	                    item->setItemStatus(ITEM_ENABLE);
	                    item->setSelfServiceBtnEnable(false);
	                    item->setContextMenuStatus((SUPPORT_PROTOCAL)data.appData->displayprotocol);
                    //}
                    //else
                    //{
                       // item->setItemStatus(ITEM_UNABLE);
                       // item->setSelfServiceBtnEnable(false);
                   // }
	                break;
	            }
	            default:
	            break;
	        }

    }

    _adjustItems();
}

void DesktopListDialog::_adjustItems()
{
    //Judge whether adjust the items or not.
    int firstItem = -1;
    int lastItem = -1;
    bool bNeedAdjust = false;
    int item_size = m_desktopItemVector.size();

    for(int i=0; i<item_size; ++i)
    {
        if(m_desktopItemVector[i]->getData().ui_status.item_status == ITEM_UNABLE)
        {
            firstItem=i;
            break;
        }
    }

    for(int i=item_size-1; i>=0; i--)
    {
         if(m_desktopItemVector[i]->getData().ui_status.item_status >= ITEM_ENABLE)
         {
             lastItem=i;
             break;
         }
    }

    if((firstItem!=-1 && lastItem!=-1) && (firstItem<lastItem))
        bNeedAdjust = true;

    //Adjust the items, put the unable item in bottom
    int count = 0 ;
    for(int i=0; (i<m_desktopItemVector.size()&& bNeedAdjust); ++i)
    {
        if(m_desktopItemVector[i]->getData().ui_status.item_status ==ITEM_UNABLE)
        {
            CDesktopListItem *item = m_desktopItemVector[i];
            m_scrollAreaLayout->removeWidget(item);
            m_desktopItemVector.remove(i);
            addDesktopItem(item);
            for(int idx = i; idx<m_desktopItemVector.size(); idx++)
            {
                if(NULL != m_desktopItemVector[idx])
                    m_desktopItemVector[idx]->hideSelfServiceDlg(true);//the item has changed it's position. so if the SelfServiceDlg is shown.hidden it
            }
            i--;
        }
        count++;
        if(m_desktopItemVector.size()==count)
            break;
    }
}

void DesktopListDialog::addDesktopItem(CDesktopListItem *item)
{
    m_scrollAreaLayout->insertWidget(m_desktopItemVector.size(), item);
     m_desktopItemVector.append(item);
}

void DesktopListDialog::__clearDesktopList()
{
    int i;
    for(i=0; i<m_desktopItemVector.size(); ++i)
    {
        m_scrollAreaLayout->removeWidget(m_desktopItemVector[i]);
        delete m_desktopItemVector[i];
    }
    m_desktopItemVector.clear();
}

void DesktopListDialog::__removeDesktopItem(QString uuid)
{
    int refrence=0;
    CDesktopListItem *item;
    item = getDesktopItem(uuid, refrence);
    if(item!=NULL)
    {
        m_scrollAreaLayout->removeWidget(item);
        delete item;
        m_desktopItemVector.remove(refrence);
    }

}


void DesktopListDialog::on_pushButton_terminalCtl()
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        return;
    }
    if( NULL != m_terminalDesktopList)
    {
        delete m_terminalDesktopList;
        m_terminalDesktopList = NULL;
    }
    int terminal_size = m_appbaklist.size();
    cout << "m_terminalList.size():" << terminal_size << endl;
    std::vector<APP_LIST> terminal_list;
    for( int loop = 0; loop < terminal_size; loop++)
    {
        if(m_appbaklist[loop].desktopType == TERMINAL&&m_appbaklist[loop].state)
        {
            terminal_list.push_back(m_appbaklist[loop]);
        }
    }
    cout << "m_terminalList.size():" << terminal_list.size() << endl;
    m_terminalDesktopList = new TerminaldesktopList(terminal_list, this);
    m_terminalDesktopList->exec();
}

void DesktopListDialog::on_pushbutton_requestBtn()
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        return;
    }
    m_requestDesktop = new RequestDesktopDlg(this);
    m_requestDesktop->exec();
}

void DesktopListDialog::on_pushButton_settings_clicked()
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        return;
    }
    std::vector<VIRTUALDISK> iHasVDisk = m_vDisks;
    if(NULL != m_personalSettingDlg)
    {
        delete m_personalSettingDlg;
        m_personalSettingDlg = NULL;
    }
    m_personalSettingDlg = new CPersonalSetting(iHasVDisk, this);
    m_personalSettingDlg->exec();
    if( m_vDisks.size() <= 0)
    {
        m_vDisks = m_personalSettingDlg->setVDisk();
    }
    if(!m_isLogout)
        m_personalSettingDlg->getVClientSettings(m_vClientSettings);
}

void DesktopListDialog::logoutSession(int flag /*= BLOCKED*/)
{
    LOG_INFO("%s","before delete m_launchApp");
    if(NULL != m_launchApp)
    {
        CLaunchApp *launchApp = m_launchApp;
        m_launchApp = NULL;
        delete launchApp;//if the network is not good. it may spent a lot of time
    }
    LOG_INFO("%s","has delete m_launchApp");
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = MAINWINDOW;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = flag;
    param.taskUuid = taskUuid;
    m_pSession->logoutSession(param);
}



void logff(void* p)
{
    DesktopListDialog* pDeskDlg =(DesktopListDialog* ) p;
    if(NULL == p)
    {
        LOG_ERR("%s", "NULL == p");
        return;
    }
   // if(!(pDeskDlg->getIsLogout()))
   // {
//        pDeskDlg-> setLogutProgress(1);
//        pDeskDlg->setLogout(true);
        pDeskDlg->logoutSession(UNBLOCK);
   // }
}



void DesktopListDialog::on_pushButton_logout_clicked()
{
    if(m_pSession==NULL)
        return;
    if(CMessageBox::SelectedBox(tr("Are you sure to change user?")) == REJECTED)
        return;

    //logff_slot();
    if(!m_isLogout)

    {
        setLogutProgress(1);
        m_isLogout = true;
        if(m_process != NULL)
        {
            m_process->termate();
            delete m_process;
            m_process = NULL;
        }
        if(m_IpcItalc != NULL)
        {
            delete m_IpcItalc;
            m_IpcItalc = NULL;
        }
        int iRet = CThread::createThread(NULL, NULL, (FUN_THREAD)(&logff), (void*)this);
        if(iRet < 0)
        {
            LOG_ERR("%s", "create thread failed.");
        }
    }

}

void DesktopListDialog::logff_slot()
{
    if(!m_isLogout)
    {
        setLogutProgress(1);
        m_isLogout = true;
        logoutSession(UNBLOCK);
    }
}
/*************************************************************
 * IPC message functions
 **/

DesktopListDialog *DesktopListDialog::s_desktopListDlg = NULL;
QMutex DesktopListDialog::s_mutex;

void DesktopListDialog::ipcClientProcessMsg(void *ipcMsg)
{
	if( ipcMsg != NULL){
		LOG_INFO("the logout msg: %s", (char*)(ipcMsg));
	}
    QMutexLocker mutexLocker(&s_mutex);
    if(s_desktopListDlg != NULL)
    {
        s_desktopListDlg->ipc_logoff();
    }
}

void DesktopListDialog::ipc_logoff()
{
    emit on_signal_logoff();
}

////////////////////////////////////////////////////////

void DesktopListDialog::on_update_resource_list(LIST_USER_RESOURCE_DATA appList,
                                                GET_USER_INFO_DATA vDisks,
                                                GET_RESOURCE_PARAMETER resParam_out)
{///ATTENTION use LOCKS!!!!!!!!!
    if(m_isLogout)
        return;
//    qDebug() << "appList size: " <<(int)appList.stAppList.size();
    m_mutex.lock();// mainly lock for m_appList
    bool removeFlag = false;
    vector<APP_LIST> &stAppList = appList.stAppList;
    vector<APP_LIST> &stAppBakList = appList.stAppBakList;

    /*
     *note: refresh m_appbaklist; don't need delete item;
     */
    if(stAppBakList.size() <= 0)
    {
        m_appbaklist.clear();
    }
    else
    {
        m_appbaklist.clear();
        for(std::vector<APP_LIST>::size_type j = 0; j < stAppBakList.size();  j++)
        {
            m_appbaklist.push_back(stAppBakList.at(j));

        }
    }
    /*
     *note: refresh m_applist;
     */
    if(stAppList.size()<=0)
    {
        for(std::vector<APP_LIST>::size_type i=0; i<m_appList.size(); ++i)
        {
            __removeDesktopItem(QString(m_appList[i].uuid));
        }
        qDebug()<<"m_appList.clear()"<<m_appList.size();
        m_appList.clear();
    }
    else
    {
        for(std::vector<APP_LIST>::size_type i=0; i<m_appList.size(); ++i)
        {
            APP_LIST appdata = m_appList.at(i);
            if(stAppList.size()<=0)
                removeFlag = true;
            else
            {
                for(std::vector<APP_LIST>::size_type j=0; j<stAppList.size(); ++j)
                {
//                  qDebug() << "j= "<<j <<"size-1= "<< stAppList.size()-1 <<"uuid="<<stAppList.at(j).uuid;
                    if(strcmp(stAppList.at(j).uuid, appdata.uuid)==0)
                    {
//                      qDebug() << "modify the desktop item data: " << QString(appdata.uuid);
                        m_appList[i] = stAppList.at(j);
                        stAppList.erase(stAppList.begin() + j);
                        break;
                    }
                    else if(j==(stAppList.size()-1))
                        removeFlag = true;
//                   qDebug() << "reMoveFlag="<<removeFlag;
                }
            }

            if(removeFlag)// || (stAppList.size()==0 && i!= (m_appList.size()-1)))
            {
                //qDebug() << "remove the desktop item: " << QString(appdata.uuid)<<"\t"<<appdata.name;
                LOG_INFO("remove desktop item:uuid:%s, name:%s", appdata.uuid, appdata.name);
                __removeDesktopItem(QString(appdata.uuid));
                m_appList.erase(m_appList.begin() + i);
                i--;
                removeFlag = false;
            }
        }
    }

//    for(int i=0; i<m_appList.size(); ++i)
//    {
//        qDebug() << "desktop name: "<< m_appList[i].name << "vmstate: "<<m_appList[i].vmState << "rdpState: "<< m_appList[i].rdpServiceState;
//    }


    //Add the surplus items
    for(std::vector<APP_LIST>::size_type k=0; k<stAppList.size(); ++k)
    {
        m_appList.push_back(stAppList.at(k));
        if(stAppList[k].desktopType == VIRTUALAPP)
        {
            getAppIcon(stAppList[k].uuid);
        }
    }

    _createTableWidgetItem();

    if(strlen(resParam_out.deskUuid)>0)
    {
        ITEM_DATA data;
        CDesktopListItem *item = getDesktopItem(resParam_out.deskUuid);
        if(NULL== item)
        {
            LOG_ERR("%s", "NULL== item");
            m_mutex.unlock();
            return;
        }
        APP_LIST * pAppList =item->getData().appData;
        if(NULL!= pAppList)
        {
            pAppList->resParams = resParam_out.stResPara;
        }
        else
        {
            LOG_ERR("%s","NULL== pAppList");
        }
    }
    else
    {
        LOG_ERR("%s", "strlen(resParam_out.deskUuid)<=0");
    }
    //m_vDisks = vDisks.vstVirtualDisks;

    m_mutex.unlock();

}

void DesktopListDialog::on_keepSession_faield(int errorCode, int dType)
{
    LOG_ERR("%s", "Keep session or refresh resource error , ui logout");
    if(m_isLogout)
        return;

    disconnect(m_refreshThread, SIGNAL(on_signal_update_failed(int, int)), this, SLOT(on_keepSession_faield(int, int)));
    m_isLogout = true;

    //in this condition, we will close the DesktopListDialog, but if CPersonalSetting or DesktopSettingDialog
    //is showing , it will not disappear
    if(NULL != m_desktopSettingDlg)
        m_desktopSettingDlg->close();
    if(NULL != m_personalSettingDlg)
        m_personalSettingDlg->close();
    if( NULL != m_terminalDesktopList)
        m_terminalDesktopList->close();
    m_mutex_disconnectDesktop.lock();
    for(QHash<QString, st_reAttachVDisk_Dlg_Info>::iterator iter=m_hashSelectDlg.begin(); iter != m_hashSelectDlg.end();)
    {
        st_reAttachVDisk_Dlg_Info stVdiskDlgInfo= iter.value();
        if(NULL != stVdiskDlgInfo.pSelDlg)
        {
            LOG_INFO("%s","NULL != pSelectDlg going to close");
            stVdiskDlgInfo.pSelDlg->close();
            LOG_INFO("%s","NULL != pSelectDlg close finished.");
            if(NULL != m_launchApp)
            {
                m_launchApp->reloadVDiskInNewDesktop(iter.key().toStdString(), ONLY_RETURN, stVdiskDlgInfo.launchType);
            }
        }
        m_hashSelectDlg.erase(iter++);
    }
    m_mutex_disconnectDesktop.unlock();

    on_logout_finish(errorCode, dType);
    switch(errorCode)
    {
    case ERROR_FAIL:
        if(dType == TYPE_KEEPSESSION)
            CMessageBox::CriticalBox(tr("Keep Session failed"));
        else
            CMessageBox::CriticalBox(tr("Refresh resource list failed"));
        break;
    default:
        showUiErrorCodeTip(errorCode);
        break;
    }
}

void DesktopListDialog::on_logout_finish(int errorCode ,int dType)
{
     cerr << "why we got here. DesktopListDialog::on_logout_finish" << endl;

     LOG_ERR("Logout errorCode:%d, Type: %d\n", errorCode, dType);

    if(m_refreshThread != NULL)
    {
        m_refreshThread->setStop(true);
        m_refreshThread->wait();
    }

    m_mutex_disconnectDesktop.lock();
    if(NULL != m_launchApp)
    {
        CLaunchApp *launchApp = m_launchApp;
        m_launchApp = NULL;
        delete launchApp;
    }
    m_mutex_disconnectDesktop.unlock();

    m_mutex.lock();
    for(QHash<QString, taskUUID>::iterator iter=m_hashGetIconThreadUuid.begin(); iter != m_hashGetIconThreadUuid.end();)
    {
        if(NULL != m_pSession)
        {
            m_pSession->cancelTask(iter.value());
            LOG_INFO("has erase m_hashGetIconThreadUuid, key:value:%s,%d", iter.key().toUtf8().data(), iter.value());
        }
        m_hashGetIconThreadUuid.erase(iter++);
    }
    m_mutex.unlock();
    if(NULL != m_pSession)
    {
        m_pSession = NULL;
        CSession::Release();
    }

    close();
    UserLoginDlg *loginDlg = new UserLoginDlg(true, false);
    loginDlg->show();
}

void DesktopListDialog::on_switch_access_finish(int errorCode ,int dType)
{
    cerr << "we are in the right slog now" << endl;

     LOG_ERR("Logout errorCode:%d, Type: %d\n", errorCode, dType);

    if(m_refreshThread != NULL)
    {
        m_refreshThread->setStop(true);
        m_refreshThread->wait();
    }

    m_mutex_disconnectDesktop.lock();
    if(NULL != m_launchApp)
    {
        CLaunchApp *launchApp = m_launchApp;
        m_launchApp = NULL;
        delete launchApp;
    }
    m_mutex_disconnectDesktop.unlock();

    m_mutex.lock();
    for(QHash<QString, taskUUID>::iterator iter=m_hashGetIconThreadUuid.begin(); iter != m_hashGetIconThreadUuid.end();)
    {
        if(NULL != m_pSession)
        {
            m_pSession->cancelTask(iter.value());
            LOG_INFO("has erase m_hashGetIconThreadUuid, key:value:%s,%d", iter.key().toUtf8().data(), iter.value());
        }
        m_hashGetIconThreadUuid.erase(iter++);
    }
    m_mutex.unlock();
    if(NULL != m_pSession)
    {
        m_pSession = NULL;
        CSession::Release();
    }

    close();

    UserLoginDlg *loginDlg = new UserLoginDlg(true, true);
    loginDlg->show();
}


void DesktopListDialog::on_start_desktop_clicked(QString uuid)
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        return;
    }
    qDebug() << "start desktop: "<< uuid;
    int refrence;
    CDesktopListItem *item = getDesktopItem(uuid, refrence);
    if(item==NULL)
        return;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    item->setBusy(true);
    item->getData().ui_status.isStarting = true;
    item->setSelfServiceStatus(-1);
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = MAINWINDOW;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    if(item->getData().appData->desktopType == DESKTOPPOOL)
        m_pSession->startDesktopPool(param, uuid.toLocal8Bit().data());
    else
        m_pSession->powerOnDesktop(param, uuid.toLocal8Bit().data());
}

void DesktopListDialog::on_restart_desktop_clicked(QString uuid)
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        return;
    }
    qDebug() << "reboot desktop: "<< uuid;
    int reference;
    CDesktopListItem *item = getDesktopItem(uuid, reference);
    if(item==NULL)
        return;
    item->setBusy(true);
    item->getData().ui_status.isReStarting = true;
    item->setSelfServiceStatus(-1);
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = MAINWINDOW;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK; // set  UNBLOCK
    param.taskUuid = taskUuid;
    if(item->getData().appData->desktopType == DESKTOPPOOL)
        m_pSession->restartDesktopPool(param, uuid.toLocal8Bit().data());
    else
        m_pSession->rebootDesktop(param, uuid.toLocal8Bit().data());
}

void DesktopListDialog::on_shutdown_desktop_clicked(QString uuid)
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        return;
    }
    qDebug() << "shutdown desktop: "<< uuid;
    int reference;
    CDesktopListItem *item = getDesktopItem(uuid, reference);
    if(item == NULL)
        return;
    item->setBusy(true);
    item->getData().ui_status.isShutdown = true;
    item->setSelfServiceStatus(-1);
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = MAINWINDOW;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    if(item->getData().appData->desktopType == DESKTOPPOOL)
        m_pSession->stopDesktopPool(param, uuid.toLocal8Bit().data());
    else
        m_pSession->poweroffDesktop(param, uuid.toLocal8Bit().data());
}

void DesktopListDialog::on_set_desktop_clicked(QString uuid)
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        return;
    }
    qDebug() << "config desktop: "<< uuid;
    desktopSettingParameter param;
    memset(&param, 0, sizeof(DESKTOP_SETTING_PARAM));
    if(m_vDisks.size()>0)
        param.hasVirtualDisk=true;
    param.item = getDesktopItem(uuid);
    if(NULL != m_desktopSettingDlg)
    {
        delete m_desktopSettingDlg;
        m_desktopSettingDlg = NULL;
    }
    m_desktopSettingDlg = new DesktopSettingDialog(param, param.item);
    m_desktopSettingDlg->exec();
}

void DesktopListDialog::on_disconnect_desktop_clicked(QString uuid)
{// this lock is unlocked when disconnect finished....
   // m_mutex_disconnectDesktop.lock(); //comment out by qc, we now needn't this?
                                                             //otherwise, when vCenter is down, and we disconnect two FAP at the same time, gui will freeze.
    if(NULL != m_launchApp)
    {
        CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
        if(NULL == pCall_param)
        {
            LOG_ERR("%s", "new failed! NULL == pCall_param");
            //m_mutex_disconnectDesktop.unlock();
            return;
        }

        // [
        //add by qc
        int reference;
        CDesktopListItem *item = getDesktopItem(uuid, reference);
        if(item==NULL)
            return;
        item->setBusy(true);
        item->getData().ui_status.isDisconnecting = true;
        item->setSelfServiceStatus(-1);
       // add by qc
        //]


        pCall_param->pUi = this;
        pCall_param->uiType = MAINWINDOW;
        memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));

        DISCONN_APP stDisConnApp;
        stDisConnApp.strDesktopUuid = uuid.toStdString();
        stDisConnApp.pCallbackUi_param = pCall_param;
        stDisConnApp.pFunUi = uiCallBackFunc;
        stDisConnApp.isBlock = UNBLOCK;

        cout << "desktoplsit::disconnectapp" << endl;
        m_launchApp->disconnectApp(stDisConnApp);//(uuid.toStdString());
    }
    else
    {
        //m_mutex_disconnectDesktop.unlock();//comment out by qc, we now needn't this?
                                                                   //otherwise, when vCenter is down, and we disconnect two FAP at the same time, gui will freeze.
        LOG_ERR("%s", "NULL == m_launchApp");
    }    
}

void DesktopListDialog::on_disconnect_desktop_finished(QString uuid)
{
    // [
    // added by qc
    int reference;
    CDesktopListItem *item = getDesktopItem(uuid, reference);
    if(item!=NULL)
    {
        ITEM_DATA &data= item->getData();
        data.ui_status.isDisconnecting=false;
        item->setItemStatus(ITEM_ENABLE);
        item->setSelfServiceStatus(0);
        item->setBusy(false);
        item->hideSelfServiceDlg(true);

        _adjustItems();//the status has changed, need to adjust
    }
    // added by qc
    //]

    //int reference;
    //CDesktopListItem *item = getDesktopItem(uuid, reference);
    //if(item!=NULL)
       // if(item->getData().ui_status.item_status == ITEM_CONNECTED)
            //item->setItemStatus(ITEM_ENABLE);

    // m_mutex_disconnectDesktop.unlock(); //comment out by qc, we now needn't this?
                                                                 //otherwise, when vCenter is down, and we disconnect two FAP at the same time, gui will freeze.

}

void DesktopListDialog::on_setDefaultApp(int status, LAUNCH_TYPE launchType, APP_LIST stAppList)
{
    if(status < 0)
    {
        CMessageBox::CriticalBox(tr("set default desktop(application) failed."));
    }
    else if(status == 1)
    {//set as default app
        SETTING_DEFAULTAPP default_desktop;
        g_pConfigInfo->getSetting_defaultDesktop(default_desktop);

        strcpy(default_desktop.uuid, stAppList.uuid);
        strcpy(default_desktop.appName,  stAppList.name);
        strcpy(default_desktop.serverIp, m_pSession->getNetwork().stPresentServer.serverAddress);//strcpy(default_desktop.serverIp, m_pSession->getNetwork().presentServer);
        strcpy(default_desktop.userName, m_pSession->getUSER_INFO().username);
        if(LAUNCH_TYPE_RDP == launchType)
            default_desktop.connectProtocal = 0;
        if(LAUNCH_TYPE_FAP == launchType)
            default_desktop.connectProtocal = 1;
        else if (LAUNCH_TYPE_TERMINAL == launchType)
            default_desktop.connectProtocal = 2;

        default_desktop.isAutoConnect = 1;
        //default_desktop.isLoadvDisk = ui->checkBox_virtualdisk->isChecked();
        default_desktop.isMapUsb = 0;
        g_pConfigInfo->setSetting_defaultDesktop(default_desktop);
    }
    else if(status == 2)
    {//cancel default app set
        SETTING_DEFAULTAPP default_desktop;
        memset(&default_desktop, 0, sizeof(SETTING_DEFAULTAPP));
        g_pConfigInfo->setSetting_defaultDesktop(default_desktop);

    }
    else
    {
        LOG_ERR(" unknown op type:%d", status);
    }
}

void DesktopListDialog::on_selfService_finished(int errorCode, int dType, QString uuid, int vmState, int rdpState)
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        return;
    }
    LOG_INFO("%s", "self service finished...........................................");
    if(errorCode != 0)
    {
        LOG_INFO("%s", "self service error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        cerr << "we got here: desktoplistdialog.cpp line 1311" << endl;
        processErrorCode(errorCode, dType);
        if(m_isLogout)
        {
            LOG_INFO("%s","has logout......");
            return;
        }
    }
    int reference;
    CDesktopListItem *item = getDesktopItem(uuid, reference);
    if(item!=NULL)
    {
#ifdef VERSION_JSJQ
        m_mutex_selfservice.lock();
        ITEM_DATA &data= item->getData();
        data.ui_status.isReStarting=false;
        data.ui_status.isStarting=false;
        data.ui_status.isShutdown=false;
        data.appData->vmState=vmState;
        data.appData->rdpServiceState=rdpState;
        if(vmState>0 || rdpState>0)
        {
            LOG_INFO("ITEM_ENABLE");
            item->setItemStatus(ITEM_ENABLE);
            item->setSelfServiceStatus(0);
        }
        else
        {
            LOG_INFO("ITEM_UNABLE");
            item->setItemStatus(ITEM_UNABLE);
            item->setSelfServiceStatus(1);
        }
        item->setBusy(false);
        item->hideSelfServiceDlg(true);
        if((dType == TYPE_START_DESKTOPPOOL || dType == TYPE_POWERON_DESKTOP) && errorCode == 0)
        {
            for(int loop =0; loop < m_desktopItemVector.size() ; loop++)
            {
                if(m_desktopItemVector[loop]->getData().appData->desktopType == DESKTOPPOOL && strcmp(m_desktopItemVector[loop]->getData().appData->uuid, item->getData().uuid.toStdString().c_str()) !=0 )
                {
                    if(m_desktopItemVector[loop]->getData().appData->sourceType==0)
                    {
                        if(m_desktopItemVector[loop]->getData().appData->userAssignment==0 || m_desktopItemVector[loop]->getData().appData->userAssignment ==2)
                        {
                            if(m_desktopItemVector[loop]->getData().appData->vmState>0 || m_desktopItemVector[loop]->getData().appData->rdpServiceState>0)
                            {
                                on_shutdown_desktop_clicked(m_desktopItemVector[loop]->getData().uuid);
                                LOG_INFO("desktoppool : %s",m_desktopItemVector[loop]->getData().appData->name);
                            }
                        }
                    }
                }else if(m_desktopItemVector[loop]->getData().appData->desktopType == REMOTEDESKTOP && strcmp(m_desktopItemVector[loop]->getData().appData->uuid, item->getData().uuid.toStdString().c_str()) !=0 )
                {
                    if(m_desktopItemVector[loop]->getData().appData->vmState>0 || m_desktopItemVector[loop]->getData().appData->rdpServiceState>0)
                    {
                        on_shutdown_desktop_clicked(m_desktopItemVector[loop]->getData().uuid);
                        LOG_INFO("remotedesktop : %s",m_desktopItemVector[loop]->getData().appData->name);
                    }
                }
            }
        }
        m_mutex_selfservice.unlock();
#else
        ITEM_DATA &data= item->getData();
        data.ui_status.isReStarting=false;
        data.ui_status.isStarting=false;
        data.ui_status.isShutdown=false;
        data.appData->vmState=vmState;
        data.appData->rdpServiceState=rdpState;
        if(vmState>0 || rdpState>0)
        {
            LOG_INFO("ITEM_ENABLE");
            item->setItemStatus(ITEM_ENABLE);
            item->setSelfServiceStatus(0);
        }
        else
        {
            LOG_INFO("ITEM_UNABLE");
            item->setItemStatus(ITEM_UNABLE);
            item->setSelfServiceStatus(1);
        }
        item->setBusy(false);
        item->hideSelfServiceDlg(true);
#endif
        _adjustItems();//the status has changed, need to adjust
    }    
}

CDesktopListItem* DesktopListDialog::getDesktopItem(QString uuid, int &vectorNum)
{
    CDesktopListItem *item = NULL;
    for(int i=0; i<m_desktopItemVector.size(); ++i)
    {
        if(uuid == m_desktopItemVector[i]->getData().uuid)
        {
            item = m_desktopItemVector[i];
            vectorNum = i;
            break;
        }
    }
    return item;
}

CDesktopListItem* DesktopListDialog::getDesktopItem(QString uuid)
{
    int num;
    return getDesktopItem(uuid, num);
}
void DesktopListDialog::on_launch_italc()
{
    VCLIENT_THREAD thread;
    VCLIENT_THREAD_CREATE(thread, NULL, launchTerminal, this);
    THREAD_DETACH(thread);
}

void *DesktopListDialog::launchTerminal(void *arg)
{
    DesktopListDialog *dlg = static_cast<DesktopListDialog *>(arg);
    if(dlg == NULL|| dlg->m_process != NULL)
    {
        return NULL;
    }
    if(dlg->m_IpcItalc != NULL)
    {
        delete dlg->m_IpcItalc;
        dlg->m_IpcItalc = NULL;
    }
    dlg->m_IpcItalc = new IpcItalc;
    dlg->m_process = new CProcessOp(APP_NAME_TERMINAL, APP_NAME_TERMINAL);
    if(dlg->m_process == NULL)
    {
        if(dlg->m_IpcItalc != NULL)
        {
            delete dlg->m_IpcItalc;
            dlg->m_IpcItalc = NULL;
        }
        return NULL;
    }
    dlg->ui->pushBtn_eClass->setCursor(Qt::WaitCursor);
//    dlg->ui->pushBtn_eClass->setEnabled(false);
    int iRet = dlg->m_process->create();
    if(iRet < 0)
    {
        dlg->ui->pushBtn_eClass->setCursor(Qt::ArrowCursor);
//        dlg->ui->pushBtn_eClass->setEnabled(true);
        if(dlg->m_IpcItalc != NULL)
        {
            delete dlg->m_IpcItalc;
            dlg->m_IpcItalc = NULL;
        }
        return NULL;
    }
    dlg->m_process->wait();
    dlg->ui->pushBtn_eClass->setCursor(Qt::ArrowCursor);
// dlg->ui->pushBtn_eClass->setEnabled(true);
    if(dlg->m_process != NULL)
    {
        delete dlg->m_process;
        dlg->m_process = NULL;
    }
    if(dlg->m_IpcItalc != NULL)
    {
        delete dlg->m_IpcItalc;
        dlg->m_IpcItalc = NULL;
    }
    return NULL;
}

void DesktopListDialog::on_launch_desktop(QString uuid)
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        return;
    }
    int reference;
    CDesktopListItem *item = getDesktopItem(uuid, reference);
    if(item == NULL)
        return;
    if(item->getData().ui_status.isStarting)
    {
        CMessageBox::CriticalBox(tr("The desktop is starting, so unable to launch"));
        return;
    }
    else if(item->getData().ui_status.isReStarting)
    {
        CMessageBox::CriticalBox(tr("The desktop is restarting, so unable to launch"));
        return;
    }
    else if(item->getData().ui_status.isShutdown)
    {
        CMessageBox::CriticalBox(tr("The desktop is stoping, so unable to launch"));
        return;
    }
    if(CSession::GetInstance()==NULL)
        return;

    if(m_launchApp==NULL)
        m_launchApp = new CLaunchApp();

    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return;
    }
    //get loadvDisk info
    SETTINGS_LOGIN stLoginSetting;
    memset((void*)(&stLoginSetting), 0, sizeof(SETTINGS_LOGIN));
    int iRet = g_pConfigInfo->getSettings_login(stLoginSetting);
    if(iRet < 0)
        LOG_ERR("get config info from file failed. return value:%d", iRet);

    item->setItemStatus(ITEM_CONNECTED);
    if(item->getData().appData->powerOnVmNum>0 || item->getData().appData->rdpOnVmNum>0)
        item->setSelfServiceStatus(4);

    pCall_param->pUi = this;
    pCall_param->uiType = MAINWINDOW;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));

    PARAM_LAUNCH_COMMON_IN stLaunchCommon;
    stLaunchCommon.isBlock = UNBLOCK;
    stLaunchCommon.pFunUi = uiCallBackFunc;
    stLaunchCommon.pCallbackUi_param = pCall_param;
    //LAUCH_RDP stLaunchRdp;
    if(item->getData().ui_status.connectType == RDP_CONNECT)
    {
        LAUCH_RDP stLaunchRdp;
       // stLaunchRdp.isConnected = item->getData().ui_status.isConnected;
        stLaunchRdp.barStatus = m_vClientSettings.m_rdpBar;
        if(UNKNOWN_BAR_STATUS == stLaunchRdp.barStatus)
            stLaunchRdp.barStatus = HASBAR_STATE;
        stLaunchRdp.stAppInfo = *(item->getData().appData);
        stLaunchRdp.stPort = item->getData().ui_status.stPort;
        stLaunchRdp.bMapFileSystem = false;
        if( m_vDisks.size() > 0)
        {
            stLaunchRdp.launchDisk = m_vDisks.at(0);
            LOG_INFO("m_vDisks:%s", stLaunchRdp.launchDisk.devicePath);
        }

//        stLaunchRdp.bMapUsb = false;
        //stLaunchFap.bMapUsb = g_isUsbOccupy; //item->getData().ui_status.hasUsbMapping;//
//        if(!g_isUsbOccupy )//&& strlen(item->getData().appData->resParams.usb)>0
//        {
//            g_isUsbOccupy = true;
//            stLaunchRdp.bMapUsb = true; //item->getData().ui_status.hasUsbMapping;//
//            item->getData().ui_status.hasUsbMapping = true;
//            LOG_INFO("map usb. desktopname:%s, deskuuid:%s",item->getData().appData->name, item->getData().appData->uuid);
//        }
//        else
//            item->getData().ui_status.hasUsbMapping = false;

        /***************************
         *  Note:
         *Now the file system can only be mapped by one connected desktop in unix.
         *so the code below about mapping file system is avilable in unix.
         *
         ***************************/
        if(!g_bMapFileSystemOccupy )// && item->getData().appData->resParams.disk==1)
        {
            g_bMapFileSystemOccupy = true;
            stLaunchRdp.bMapFileSystem = true;
            item->getData().ui_status.bHasFileSystemMapping = true;
        }
        if((item->getData().appData->desktopType==DESKTOPPOOL && item->getData().appData->sourceType==0)
                || item->getData().appData->desktopType ==REMOTEDESKTOP)
        {
            if(m_vDisks.size()<=0)
            {
                LOG_ERR("%s","the user doesnot have any vDisks");
                stLaunchRdp.bAttachDisk = 0;
            }
            else
                stLaunchRdp.bAttachDisk = stLoginSetting.iAttachVDisk==0?false:true;//item->getData().ui_status.hasVDisk;
        }
        else
            stLaunchRdp.bAttachDisk = 0;
        m_launchApp->launchRdp(stLaunchCommon, stLaunchRdp);
    }
    else if (item->getData().ui_status.connectType == FAP_CONNECT)
    {
        LAUCH_FAP stLaunchFap;
        memset(&stLaunchFap, 0, sizeof(LAUCH_FAP));

        stLaunchFap.mapStatus = m_vClientSettings.m_mapset;
        if(UNKNOWN_MAP_STATUS == stLaunchFap.mapStatus){
            stLaunchFap.mapStatus = NOMAP_STATE;
            stLaunchFap.bMapFileSystem = false;
        }else if( MAP_STATE == stLaunchFap.mapStatus){
            if(!g_bMapFileSystemOccupy )
            {
                g_bMapFileSystemOccupy = true;
                stLaunchFap.bMapFileSystem = true;
                item->getData().ui_status.bHasFileSystemMapping = true;
                strcpy(stLaunchFap.stFileInfo.stFirstPath.filePath,m_vClientSettings.m_mapFilePathList.stFirstPath.filePath);
                LOG_INFO("filepath: %s===", m_vClientSettings.m_mapFilePathList.stFirstPath.filePath);
            }
        }
//        stLaunchFap.isConnected = item->getData().ui_status.isConnected;
        stLaunchFap.barStatus = m_vClientSettings.m_fapBar;
        if(UNKNOWN_BAR_STATUS == stLaunchFap.barStatus)
            stLaunchFap.barStatus = HASBAR_STATE;
        stLaunchFap.stAppInfo = *(item->getData().appData);
        stLaunchFap.stPort = item->getData().ui_status.stPort;
        if( m_vDisks.size() > 0)
        {
            stLaunchFap.launchDisk = m_vDisks.at(0);
        }
//        stLaunchFap.bMapUsb = false;
//        //stLaunchFap.bMapUsb = g_isUsbOccupy; //item->getData().ui_status.hasUsbMapping;//
//        if(!g_isUsbOccupy )//&& strlen(item->getData().appData->resParams.usb)>0
//        {
//            g_isUsbOccupy = true;
//            stLaunchFap.bMapUsb = true; //item->getData().ui_status.hasUsbMapping;//
//            item->getData().ui_status.hasUsbMapping = true;
//            LOG_INFO("map usb. desktopname:%s, deskuuid:%s",item->getData().appData->name, item->getData().appData->uuid);
//        }
//        else
//            item->getData().ui_status.hasUsbMapping = false;

        if( m_vDisks.size() > 0)
        {
            stLaunchFap.launchDisk = m_vDisks.at(0);
            LOG_INFO("m_vDisks:%s", stLaunchFap.launchDisk.devicePath);
        }

        if((item->getData().appData->desktopType == DESKTOPPOOL && item->getData().appData->sourceType==0)
            ||item->getData().appData->desktopType ==REMOTEDESKTOP)
        {
            if(m_vDisks.size()<=0)
            {
                LOG_ERR("%s","the user doesnot have any vDisks");
                stLaunchFap.bAttachDisk = 0;
            }
            else
                stLaunchFap.bAttachDisk = stLoginSetting.iAttachVDisk==0?false:true;//item->getData().ui_status.hasVDisk;
        }
        else
            stLaunchFap.bAttachDisk = 0;
        m_launchApp->launchFap(stLaunchCommon, stLaunchFap);
    }
#ifndef _WIN32
    else if (item->getData().ui_status.connectType == TERMINAL_CONNECT)
    {
        LAUNCH_TERMINAL stLaunchTerminal;
        stLaunchTerminal.stAppInfo = *(item->getData().appData);
        m_launchApp->launchTerminal(stLaunchCommon, stLaunchTerminal);
    }
#endif
#ifdef VERSION_JSJQ
    showMinimized();
#endif
}

//return value: >=0, has dealed. doesnot need to
int DesktopListDialog::dealAttachDisk_desktoppool(const QString uuid, CDesktopListItem *item)
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        m_launchApp->reloadVDiskInNewDesktop(uuid.toUtf8().data(), ONLY_RETURN, (LAUNCH_TYPE)(item->getData().ui_status.connectType));
        return 0;
    }

    int iRet = 0;
    CSelectDialog* pSelectDlg = new CSelectDialog(tr("don't attach the virtual disk, connect to\nthe desktop directly."),\
                                                  tr("connect to a new desktop and try to load\nvirtual disk again."),\
                                                  tr("Attach virtual disk failed!"), tr("vClient"));
    m_mutex.lock();
    st_reAttachVDisk_Dlg_Info stVdiskDlgInfo;
    stVdiskDlgInfo.launchType = (LAUNCH_TYPE)(item->getData().ui_status.connectType);
    stVdiskDlgInfo.pSelDlg = pSelectDlg;
    m_hashSelectDlg.insert(uuid, stVdiskDlgInfo);
    m_mutex.unlock();
    pSelectDlg->exec();
    LOG_INFO("%s", "m_hashSelectDlg dlg closed");
    int iSelect = pSelectDlg->getSelection();

    bool hasFound = false;
    m_mutex.lock();
    for(QHash<QString, st_reAttachVDisk_Dlg_Info>::iterator iter=m_hashSelectDlg.begin(); iter != m_hashSelectDlg.end();)
    {
        hasFound = true;
        m_hashSelectDlg.erase(iter++);
    }
    m_mutex.unlock();
    delete pSelectDlg;
    pSelectDlg = NULL;
    if(!hasFound)
    {
        LOG_INFO("%s","hasFound is false, going to exit directly");
        return 0;
    }
    LOG_INFO("desktoppool attach disk selection:%d", iSelect);
    if(NULL==m_launchApp || NULL==item)
    {
        LOG_ERR("NULL==m_launchApp || NULL==item. item:%d, m_launchApp:%d", NULL==item?0:1, NULL==m_launchApp?0:1);
        return -1;
    }
//    if(m_isLogout)//it should not added. because in CLaunchApp there has a signal would block it to exit
//    {
//        LOG_INFO("%s","has logout......");
//        return -5;
//    }
    if(0 == iSelect)
    {
        iRet = m_launchApp->reloadVDiskInNewDesktop(uuid.toUtf8().data(), RELOAD_VDISK_NO, (LAUNCH_TYPE)(item->getData().ui_status.connectType));
    }
    else
    {
        iRet = m_launchApp->reloadVDiskInNewDesktop(uuid.toUtf8().data(), RELOAD_VDISK_YES, (LAUNCH_TYPE)(item->getData().ui_status.connectType));
    }
    return iRet;
}

void DesktopListDialog::on_launch_desktop_finished(int errorCode, int dType, QString uuid, LAUNCH_DESKTOP_DATA* launchData)
{
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        if(NULL!=launchData && launchData->iOpType==TYPE_LAUNCH_ATTACH_DISK_FAILED)// if it is float desktoppool
            ;
        else
            return;
    }

    int refrence;
    CDesktopListItem *item = getDesktopItem(uuid, refrence);
    if(NULL !=launchData )
    {
        LOG_INFO("type:%d",launchData->iOpType);
        if(launchData->iOpType == TYPE_BEGIN_ATTACH_DISK)
        {
            if(NULL != item)
                item->setBusy(true);
            delete launchData;
            launchData  = NULL;
            return;
        }
        else if(launchData->iOpType == TYPE_LAUNCH_ATTACH_DISK_FAILED)
        {
            enum desktoptype desktopType = VIRTUALAPP;
            if(NULL != item)
            {
                item->setBusy(false);
                desktopType = item->getData().appData->desktopType;
            }
            if(DESKTOPPOOL == desktopType && 1==item->getData().appData->userAssignment)
            {
//                if( 0 < m_vDisks.size() )
//                item->getData().vDisk = m_vDisks.at(0);  // refresh the virtualDisk.devicepath;
                dealAttachDisk_desktoppool(uuid, item);
            }
            else
            {
                if(errorCode!=0 && !m_isLogout)
                {
                    if(errorCode>=0)
                    {
                        LOG_ERR("errorCode:%d", errorCode);
                        errorCode = ERROR_FAIL;
                    }
                    processErrorCode(errorCode, TYPE_LAUNCH_ATTACH_DISK_FAILED);
                }
            }
            delete launchData;
            launchData  = NULL;
            return;
        }
        else if(launchData->iOpType == TYPE_LAUNCH_ATTACH_DISK_SUCCEED)
        {
            if(NULL != item)
                item->setBusy(false);
            delete launchData;
            launchData  = NULL;
            return;
        }
        else if(launchData->iOpType == TYPE_DESKTOP_NOT_AVAILABLE)
        {
            if(NULL != item)
            {
                item->setBusy(false);
                m_mutex.lock();
                APP_LIST* pAppData = item->getData().appData;
                if(NULL != pAppData)
                {//update the desktop data
                    pAppData->rdpOnVmNum = launchData->stAppInfo.rdpOnVmNum;
                    pAppData->rdpServiceState = launchData->stAppInfo.rdpServiceState;
                    pAppData->vmState = launchData->stAppInfo.vmState;
                    pAppData->powerOnVmNum = launchData->stAppInfo.powerOnVmNum;
                    pAppData->displayprotocol = launchData->stAppInfo.displayprotocol;
                }
                m_mutex.unlock();
            }
            LOG_ERR("TYPE_DESKTOP_NOT_AVAILABLE: iRet:", errorCode);
            processErrorCode(ERROR_FAIL, TYPE_DESKTOP_NOT_AVAILABLE);
            delete launchData;
            launchData  = NULL;
            errorCode = 0;
            //return;
        }
        else
        {
            LOG_ERR(" unknown type:%d",launchData->iOpType);
            delete launchData;
            launchData  = NULL;
        }
    }
    if(m_isLogout)
    {
        LOG_INFO("%s","has logout......");
        return;
    }

    if(errorCode!=0)
    {
        processErrorCode(errorCode, dType);
        if(m_isLogout)
        {
            LOG_INFO("%s","has logout......");
            return;
        }
        if(NULL != m_launchApp && NULL !=item)
        {
            std::string str_uuid;
            str_uuid = uuid.toUtf8().data();
            if(m_launchApp->appExit(str_uuid, (LAUNCH_TYPE)(item->getData().ui_status.connectType)) <=0)
            {
                LOG_INFO("desktop has not quit, return directly. desktop uuid:%s", uuid.toUtf8().data());
                return;
            }
        }
    }
    m_mutex.lock();
    item = getDesktopItem(uuid, refrence);
    if(item==NULL)
    {
        LOG_ERR("NULL==item uuid:%s", uuid.toStdString().c_str());
        m_mutex.unlock();
        return;
    }
//    if(item->getData().ui_status.hasUsbMapping)
//    {
//        g_isUsbOccupy = false;
//        item->getData().ui_status.hasUsbMapping = false;
//        LOG_INFO("map usb released. desktopname:%s, deskuuid:%s",item->getData().appData->name, item->getData().appData->uuid);
//    }

    if(item->getData().ui_status.bHasFileSystemMapping)
    {
        g_bMapFileSystemOccupy = false;
        item->getData().ui_status.bHasFileSystemMapping = false;
    }
    item->hideSelfServiceDlg(true);
    if(item->getData().appData->desktopType == DESKTOPPOOL)
    {
        if(item->getData().appData->sourceType==0)
        {
            if(item->getData().appData->vmState>0 || item->getData().appData->powerOnVmNum>0 || item->getData().appData->rdpOnVmNum>0)
                item->setItemStatus(ITEM_ENABLE);
            else
                item->setItemStatus(ITEM_UNABLE);
            if(item->getData().appData->userAssignment==0 || item->getData().appData->userAssignment==2)
            {
                if(item->getData().appData->vmState>0 || item->getData().appData->rdpServiceState>0)
                    item->setSelfServiceStatus(0);
                else if(item->getData().appData->powerOnVmNum>=0 || item->getData().appData->rdpOnVmNum>=0)
                    item->setSelfServiceStatus(-1);
                else
                    item->setSelfServiceStatus(1);
            }
        }
        else
            item->setItemStatus(ITEM_ENABLE);
    }
    else if(item->getData().appData->desktopType == REMOTEDESKTOP)
    {
        if(item->getData().appData->vmState>0)
        {
            item->setItemStatus(ITEM_ENABLE);
             item->setSelfServiceStatus(0);
        }
        else
        {
            item->setItemStatus(ITEM_UNABLE);
             item->setSelfServiceStatus(1);
        }
    }
    else
        item->setItemStatus(ITEM_ENABLE);
    m_mutex.unlock();
}

void DesktopListDialog::processErrorCode(int errorCode, int opType)
{
    LOG_ERR("errcode:%d\t\t opType:%d", errorCode, opType);
    switch(errorCode)
    {
    case ERROR_FAIL:
    {
        if(opType == TYPE_LAUNCH_RES)
            CMessageBox::CriticalBox(tr("Connect to desktop failed"));
        else if(opType==TYPE_ATTACH_DISK || opType==TYPE_LAUNCH_ATTACH_DISK_FAILED)
            CMessageBox::CriticalBox(tr("Attach virtual disk failed"));
        else if(opType == TYPE_DESKTOP_NOT_AVAILABLE)
            CMessageBox::WarnBox(tr("No available desktop"));
        else
            CMessageBox::CriticalBox(tr("Opetate failed"));
        break;
    }
    default:
        showUiErrorCodeTip(errorCode);
        break;
    }
}

void DesktopListDialog::processCallBackData(int errorCode, int dType, void *pRespondData)
{
    switch(dType)
    {
    case TYPE_START_DESKTOPPOOL:
    case TYPE_RESTART_DESKTOPPOOL:
    case TYPE_STOP_DESKTOPPOOL:
    case TPPE_CHECK_DESKTOPPOOL_STATE:
    case TYPE_POWERON_DESKTOP :
    case TYPE_REBOOT_DESKTOP:
    case TYPE_POWEROFF_DESKTOP:
    case TYPE_CHECK_REMOTEDESKTOP_STATE:
    {
        CHECK_DESK_STATE_DATA *desktopStatus = (CHECK_DESK_STATE_DATA *)pRespondData;
        QString uuid;
        int vmState = 0;
        int rdpState = 0;
        if(desktopStatus!=NULL)
        {
            qDebug() << "self service call back:" <<desktopStatus->strJobId.c_str();
            uuid = QString::fromStdString(desktopStatus->strJobId);
            vmState = desktopStatus->iState;
            rdpState = desktopStatus->iRdpState;
            delete desktopStatus;
        }
        emit on_signal_selfService_finished(errorCode, dType, uuid, vmState, rdpState);

        break;
    }
    case TYPE_LOGOUT:
    {
        LogoutData *logout_data = (LogoutData *)pRespondData;
        if(logout_data!=NULL)
            delete logout_data;
        emit on_signal_logout_finished(errorCode, dType);
        break;
    }


    case TYPE_SWITCH_ACCESS:
    {
        LogoutData *logout_data = (LogoutData *)pRespondData;
        if(logout_data!=NULL)
            delete logout_data;
        emit on_signal_switch_access_finished(errorCode, dType);
        break;
    }


    case TYPE_FRONTEND_LAUNCHDESKTOP:
    {
        LAUNCH_DESKTOP_DATA *launch_data = (LAUNCH_DESKTOP_DATA* )pRespondData;
        QString uuid;
        if(launch_data!=NULL)
        {
            uuid = QString::fromStdString(launch_data->strDesktopUuid);
            //delete launch_data;
        }
        emit on_signal_launch_desktop_finished(errorCode, dType, uuid, launch_data);
        break;
    }
    case TYPE_FRONTEND_DISCONN_APP:
    {
        LAUNCH_DESKTOP_DATA* pData = (LAUNCH_DESKTOP_DATA*)pRespondData;
        if(NULL != pData)
        {
            emit on_signal_disconn_app_finished(QString(pData->strDesktopUuid.c_str()));
            delete pData;
            pData = NULL;
        }
        break;
    }
    case TYPE_GETICON:
    {
        GET_ICON_DATA* pIconData = (GET_ICON_DATA*)pRespondData;
        if(NULL != pIconData)
        {
            m_mutex.lock();
            QHash<QString, taskUUID>::iterator iter= m_hashGetIconThreadUuid.find(pIconData->strAppUuid.c_str());
            if(m_hashGetIconThreadUuid.end() != iter)
            {
                LOG_INFO("has erase m_hashGetIconThreadUuid, key:value:%s,%d", iter.key().toUtf8().data(), iter.value());
                m_hashGetIconThreadUuid.erase(iter);
            }
            m_mutex.unlock();
        }
        emit on_signal_getIcon(errorCode, pRespondData);
        break;
    }
    }
}

void DesktopListDialog::switchAccessSession(int flag /*= BLOCKED*/)
{
    LOG_INFO("%s","before delete m_launchApp");
    if(NULL != m_launchApp)
    {
        CLaunchApp *launchApp = m_launchApp;
        m_launchApp = NULL;
        delete launchApp;//if the network is not good. it may spent a lot of time
    }
    LOG_INFO("%s","has delete m_launchApp");

    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = MAINWINDOW;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = flag;
    param.taskUuid = taskUuid;
    m_pSession->switchAccessSession(param);
}

void swith_access(void *p)
{
    DesktopListDialog* pDeskDlg =(DesktopListDialog* ) p;
    if(NULL == p)
    {
        LOG_ERR("%s", "NULL == p");
        return;
    }
    pDeskDlg->switchAccessSession(UNBLOCK);

}

void DesktopListDialog::on_comboBox_switch_activated(const QString &arg1)
{
    //arg1 is the new access ip

    if(m_pSession==NULL)
        return;


    char AccessIp[20];

    g_pConfigInfo->getAccessIp(AccessIp);

    if (0 == strcmp(AccessIp, arg1.toStdString().c_str())) {
        return;
    }

    if(CMessageBox::SelectedBox(tr("Are you sure to change server Ip?")) == REJECTED)
        return;

    // write vAccess address settings
    if (0 != strcmp(AccessIp, arg1.toStdString().c_str())) {
        g_pConfigInfo->setAccessIp(arg1.toStdString().c_str());
    }

    // change local network settings
    // change local redhat like Linux host's eth0 networksetting
    MultiAccesses *pMultiAccesses_con = new MultiAccesses;
    pMultiAccesses_con->readFile();
    int mA_size = pMultiAccesses_con->size();
    QString qstr_tmp;
    bool network_has_changed = false;
    while (mA_size > 0) {
        if( strcmp(pMultiAccesses_con->top().AccessIp, arg1.toStdString().c_str()) == 0) {
            //Let's rock & roll
            rh_ifcfg *prh_ifcfg = new rh_ifcfg;
            if ( strlen(pMultiAccesses_con->top().ip) > 0 ) {
                prh_ifcfg->get_value(QString("BOOTPROTO"), &qstr_tmp);
                if ( strcmp(qstr_tmp.toStdString().c_str(), "none") != 0) {
                    prh_ifcfg->set_value(QString("BOOTPROTO"), QString("none"));
                    network_has_changed = true;
                }
                prh_ifcfg->get_value(QString("IPADDR"), &qstr_tmp);
                if ( strcmp(qstr_tmp.toStdString().c_str(), pMultiAccesses_con->top().ip) != 0) {
                    prh_ifcfg->set_value(QString("IPADDR"), QString(pMultiAccesses_con->top().ip));
                    network_has_changed = true;
                }
            }

            if ( strlen(pMultiAccesses_con->top().netmask) > 0) {
                prh_ifcfg->get_value(QString("NETMASK"), &qstr_tmp);
                if ( strcmp(qstr_tmp.toStdString().c_str(), pMultiAccesses_con->top().netmask) != 0) {
                    prh_ifcfg->set_value(QString("NETMASK"), QString(pMultiAccesses_con->top().netmask));
                    network_has_changed = true;
                }
            }
            if ( strlen(pMultiAccesses_con->top().gateway) > 0) {
                prh_ifcfg->get_value(QString("GATEWAY"), &qstr_tmp);
                if ( strcmp(qstr_tmp.toStdString().c_str(), pMultiAccesses_con->top().gateway) != 0) {
                    prh_ifcfg->set_value(QString("GATEWAY"), QString(pMultiAccesses_con->top().gateway));
                    network_has_changed = true;
                }
            }
            if ( strlen(pMultiAccesses_con->top().dns1) > 0) {
                cerr << "dns1: " << pMultiAccesses_con->top().dns1 << endl;
                prh_ifcfg->get_value(QString("DNS1"), &qstr_tmp);
                if ( strcmp(qstr_tmp.toStdString().c_str(), pMultiAccesses_con->top().dns1) != 0) {
                    prh_ifcfg->set_value(QString("DNS1"), QString(pMultiAccesses_con->top().dns1));
                    network_has_changed = true;
                }
            }
            break;
        }
        pMultiAccesses_con->pop();
        mA_size--;
    }
    pMultiAccesses_con->~MultiAccesses();

    if (network_has_changed) {
        if (!system("ifdown eth0; ifup eth0"))
        {
            qDebug() << "rh_ifcfg network setting succeed";
        } else {
            qDebug() << "rh_ifcfg network setting failed";
        }
    }


    if(!m_isLogout)
    {
        setLogutProgress(1);
        m_isLogout = true;
        int iRet = CThread::createThread(NULL, NULL, (FUN_THREAD)(&swith_access), (void*)this);
        if(iRet < 0)
        {
            LOG_ERR("%s", "create thread failed.");
        }
    }
}
