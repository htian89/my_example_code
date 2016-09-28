#include "userlogindlg.h"
#include "ui_userlogindlg.h"
#include "titlewidget.h"
#include "ui_interact_backend.h"
#include "cmessagebox.h"
#include "desktoplistdialog.h"
//#include "csettingwindow.h"
#include "networksettingdialog.h"
#include "../common/cconfiginfo.h"
#include "../common/log.h"
#include "../common/common.h"
#include "../common/errorcode.h"
#include "../ipc/ipcclient.h"
#include "../ipc/ipcitalc.h"
#include "cloadingdialog.h"
#include "../imageconf.h"

//debug
#include <iostream>
using namespace std;

extern CConfigInfo* g_pConfigInfo; //defined in main.cpp
extern DesktopListDialog* g_desktopListDlg; //defined in desktoplistdialog.cpp
extern bool g_bSystemShutDownSignal;    //From desktoplistdialog.cpp
extern IpcClient *g_ipcClient;  //defined in main.cpp
extern IMAGE_S vclient_image;
extern bool vclass_flag;
//extern UserLoginDlg *g_userLoginDlg;

UserLoginDlg::UserLoginDlg(bool b_getDomain/* = true*/, bool b_autoLogin/*= true*/,  QWidget *parent, bool b_isConnect) :
    QDialog(parent),
    ui(new Ui::UserLoginDlg),
    m_eLoginType(NORMAL),
    m_titleWidget(NULL),
    m_pSession(NULL),
    m_bGetDomain(b_getDomain),
    m_bAutoLogin(b_autoLogin),
    m_bIsConnect(b_isConnect),
    m_bSettingDlg(false),
    m_role(2),
    m_busyMovie(NULL),
    m_isMove(false),
    m_isGettingDomain(true)
{
    m_bHasGetListInfo = false;
    m_bHasGetUserInfo = false;
    m_dlgLoading = NULL;
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint  | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);
    //setWindowIcon(QIcon(WINDOWS_ICON));
    setWindowIcon(QIcon(WINDOWS_IMG.data()));
    //m_backgroundPixmap.load(LOGIN_DLG_BACKGROUNT_IMG);
    m_backgroundPixmap.load(vclient_image.fronview_login_background.c_str());
    if (vclass_flag)
        ui->tabWidget_login->removeTab(1);

    setDialogInCenter(this);
#ifdef WIN32
    setWindowTitle("vClient");//please do not delete this, used for close the window when updates
#else
#ifdef VERSION_HXXY
     setWindowTitle("Thinview");
#else
#ifdef VERSION_SUGON_2000
    setWindowTitle("Fronview2000");
#else
#ifdef VERSION_VCLASS
    setWindowTitle("vClass");
#else
#ifdef VERSION_JSJQ
	setWindowTitle(tr("Cloud desktop platform"));
#else
    setWindowTitle("Fronview3000");
#endif
#endif
#endif
#endif
#endif
    ui->pushButton_login->setStyleSheet(STYLE_SHEET_PUSHBTN);
    ui->comboBox_domain->setStyleSheet(STYLE_SHEET_COMBO_BOX_OTHER);
    m_titleWidget = new TitleWidget(NOMAXMIZE, "", this);
#ifdef VERSION_VCLASS
    m_titleWidget->setImage("icon_close_vclass.png", CLOSE);
    m_titleWidget->setImage("icon_minimize_vclass.png", MINIMIZE);
#else
#ifdef VERSION_JSJQ
    m_titleWidget->setImage("icon_close.png", CLOSE);
    m_titleWidget->setImage("icon_minimize.png", MINIMIZE);
#else
    m_titleWidget->setImage("icon_close_login.png", CLOSE);
    m_titleWidget->setImage("icon_minimize_login.png", MINIMIZE);
#endif
#endif
    m_titleWidget->setLayout_title(0, 0, 10, 0);
    //ui->tabWidget_login->hide();
    QString style = "QPushButton{"
            "color: #333333;"
            "font: bold 14px;"
            "border-radius: 0px;"
            "selection-color:white;}"
            "QPushButton:hover{"
            "color: #111111;}"
            "QPushButton:pressed{"
            "color: #000000;}";
    ui->pushButton_about->setStyleSheet(style);
    ui->pushButton_about->setFocusPolicy(Qt::NoFocus);	//Avoid gridline when click
    ui->pushButton_settings->setStyleSheet(style);
    ui->pushButton_settings->setFocusPolicy(Qt::NoFocus);

    ui->pushButton_login->setFocus();
    ui->pushButton_login->setShortcut(QKeySequence::InsertParagraphSeparator);
    ui->pushButton_login->setShortcut(Qt::Key_Return);
    //ui->pushButton_login->setShortcut(QKeySequence::InsertParagraphSeparator);
    //ui->pushButton_login->setDefault(true);
    //ui->pushButton_login->setShortcut(Qt::Key_Enter);
    //ui->pushButton_login->setShortcut(Qt::Key_Return);

    //connect(ui->pushButton_login, SIGNAL(clicked()), this, SLOT(on_pushButton_login_clicked()));
    //connect(ui->lineEdit_username, SIGNAL(returnPressed()), ui->pushButton_login,SIGNAL(clicked()), Qt::UniqueConnection);


    //connect some slots
    connect(this, SIGNAL(updateDomainList(int , int, QStringList)), this, SLOT(on_combox_domainlist_update(int, int, QStringList)));
    connect(ui->pushButton_settings, SIGNAL(clicked()), this, SLOT(on_settings()));
    connect(this, SIGNAL(loginFinished(int,int)), this, SLOT(on_loginFinished(int,int)));


    //Initialize auto getting doamin busy movie
    m_busyMovie = new QMovie(DOMAINBUSY);
    m_busyMovie->setScaledSize(QSize(16, 16));
    ui->label_busyDomain->setMovie(m_busyMovie);
//    setGetDomainBusy(false);

    initialize();
}

UserLoginDlg::~UserLoginDlg()
{
    if(NULL != ui)
        delete ui;
    ui = NULL;
}

int UserLoginDlg::initialize()
{
    int iRet = 0;
    if(NULL != ui)
        SetUsernameValidator(ui->lineEdit_username);
//get config info from file
    if(NULL == g_pConfigInfo)
    {
        LOG_ERR("%s", "parameter error:NULL == g_pConfigInfo");
        return -1;
    }
    iRet = g_pConfigInfo->getSettings_vclient(m_vClientSettings);
    if(iRet < 0)
    {
        LOG_ERR("%s", "get config info from file failed");
        memset(&m_vClientSettings, 0, sizeof(SETTINGS_VCLIENT));
        memset(&m_loginSettings, 0, sizeof(SETTINGS_LOGIN));
        return -5;
    }
    iRet = g_pConfigInfo->getSettings_login(m_loginSettings);
    if(iRet < 0)
    {
        LOG_ERR("get config info from file failed. return value:%d", iRet);
        memset(&m_loginSettings, 0, sizeof(SETTINGS_LOGIN));
        return -6;
    }
//update ui data
    if(NULL != ui)
    {
        ui->lineEdit_username->setText(QString::fromUtf8(m_loginSettings.stUserInfo.username));
        ui->lineEdit_password->setText(m_loginSettings.stUserInfo.password);
        if(m_loginSettings.iRemember)
            ui->checkBox_rempwd->setChecked(true);
        else
            ui->checkBox_rempwd->setChecked(false);
        if(m_loginSettings.iAutoLogin)
            ui->checkBox_autolg->setChecked(true);
        else
            ui->checkBox_autolg->setChecked(false);
        if( 0 < strlen(m_loginSettings.stUserInfo.domain))
        {
            ui->comboBox_domain->addItem(QString(m_loginSettings.stUserInfo.domain));
            ui->comboBox_domain->setCurrentIndex(1);
        }
    }

//Get domain list from server
    if(m_bGetDomain)
        __initDomainList();

    return 0;
}

void UserLoginDlg::__initDomainList()
{
    //int iRet = 0;
    if(0==strlen(m_vClientSettings.m_network.stAlternateServer.serverAddress) &&0==strlen(m_vClientSettings.m_network.stAlternateServer.port)
            && 0==strlen(m_vClientSettings.m_network.stPresentServer.serverAddress) && 0==strlen(m_vClientSettings.m_network.stPresentServer.port))//if(0==strlen(m_vClientSettings.m_network.alternateServer) && 0==strlen(m_vClientSettings.m_network.firstServer))
    {
        LOG_ERR("%s", "vAccessip not configed");
        return;
    }
//get session instance and call getdomain
    m_domainTaskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    m_pSession->setSessionInfo(&(m_vClientSettings.m_network), NULL);
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = LOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;

//    setLoginProgress(1, tr("GetDomain ..."));
    m_pSession->getDomainList(param);
    m_domainTaskUuid = param.taskUuid;
    setGetDomainBusy(true);

}

void UserLoginDlg::showEvent(QShowEvent *)
{
    if ( this->isVisible())
        this->repaint();
}

void UserLoginDlg::paintEvent(QPaintEvent *)
{
    setMaximumSize(400,350);
    QPainter paint(this);
//    paint.setRenderHint(QPainter::Antialiasing, true);
//    paint.setBrush(Qt::NoBrush);
//    QPen pen;
//    QColor color(0, 0, 0, 50);
//    for(int i=5; i>=4; i--)
//    {
//        color.setAlpha(50 - (i-1)*10);
//        pen.setColor(color);
//        paint.setPen(pen);
//        paint.drawRoundRect(QRectF(1+i, 1+i, width()-7, height()-7), 3, 3);
//    }
//    for(int i=3; i>=2; i--)
//    {
//        color.setAlpha(70 - (i-1)*12);
//        pen.setColor(color);
//        paint.setPen(pen);
//        paint.drawRoundRect(QRectF(1+i, 1+i, width()-7, height()-7), 3, 3);
//    }
//    for(int i=1; i>=0; i--)
//    {
//        color.setAlpha(100 - (i-1)*20);
//        paint.setPen(color);
//        paint.drawRoundRect(QRectF(1+i, 1+i, width()-7, height()-7), 3, 3);
//    }
    paint.setBrush(QBrush(m_backgroundPixmap));
    paint.setPen(Qt::transparent);
    paint.drawRoundRect(QRectF(0,0,width(),height()),0,0);
//    paint.drawRoundRect(QRectF(0, 0, width()-6, height()-6), 3, 3);
}

void UserLoginDlg::keyPressEvent(QKeyEvent *_ke)
{
    qDebug() << _ke->key();


    if(_ke->key()==Qt::Key_Return || _ke->key()==Qt::Key_Enter)
    {

       //on_pushButton_login_clicked();  // comment this, otherwise ,when you keep pressing enter when login, it will crash.
         _ke->accept();
    }
}

void UserLoginDlg::mousePressEvent(QMouseEvent *_qm)
{
    m_isMove = true;
    m_pressPoint = _qm->pos();
}

void UserLoginDlg::mouseMoveEvent(QMouseEvent *_qm)
{
    if(_qm->buttons() == Qt::LeftButton && m_isMove)
        move(_qm->globalPos() - m_pressPoint);
}

void UserLoginDlg::mouseReleaseEvent(QMouseEvent *)
{
    m_isMove = false;
}

/*
 *  When it is logining, set "enbale" 1, the logining progress will show in Ui with the "text" input.
 *
 *Param:
 * enable :0 , hide the login progress from UI.
 *              1, show the login progress on UI.
 *
 *text:  the text show on UI when login
 */

void UserLoginDlg::setLoginProgress(uint enable, QString text)
{
    if(enable!=0 && enable!=1)
        return;
    ui->pushButton_login->setEnabled(enable^1);
    ui->pushButton_settings->setEnabled(enable^1);

    QRect rect = geometry();
    if(NULL == m_dlgLoading)
        m_dlgLoading = new CLoadingDialog(text, this);
    else
        m_dlgLoading->setText(text);
    //m_dlgLoading->setPos(0, 100);
    m_dlgLoading->setPos(0, 72,rect.width(), rect.height()-72);
    m_dlgLoading->setMovieStop(enable^1);
    m_dlgLoading->setVisible(enable);
    ui->widget_content->setVisible(enable^1);
}

/*
 * If it is getting domain from server, then set bIsBusy true
 *and then the busy movie start in UI. otherwise, set false.
 */

void UserLoginDlg::setGetDomainBusy(bool bIsBusy)
{
    m_isGettingDomain = bIsBusy;
    ui->label_busyDomain->setVisible(bIsBusy);
    if(bIsBusy)
        m_busyMovie->start();
    else
        m_busyMovie->stop();
}

/*
 * Show a message box with the error information.
 */

void UserLoginDlg::processErrorCode(int errorCode, int opType)
{
    LOG_ERR("errcode:%d\t\t opType:%d", errorCode, opType);
    if(m_dlgLoading)
    {
        m_dlgLoading->setMovieStop(true);
        m_dlgLoading->hide();
    }
    if(opType == TYPE_GETDOMAIN && (m_bIsConnect  || m_bSettingDlg))
    {
        return;
    }
    if(opType == TYPE_GETDOMAIN && m_bAutoLogin != true && m_loginSettings.iAutoLogin != true)
        return;
    switch(errorCode)
    {
    case ERROR_FAIL:
    {
        if(opType == TYPE_LOGIN)
            CMessageBox::CriticalBox(tr("Login failed"));
        else if(opType == TYPE_LIST_USER_RES)
            CMessageBox::CriticalBox(tr("Get resource list failed"));
        else if(opType == TYPE_GETUSERINFO)
            CMessageBox::CriticalBox(tr("Get user information failed"));
        else if( opType == TYPE_GET_MONITORSINFO)
            CMessageBox::CriticalBox(tr("Get monitor information failed"));
        break;
    }
    default:
        cout << "will call showUiErrorCodeTip" << endl;
        showUiErrorCodeTip(errorCode);
        break;
    }

    ui->pushButton_login->setEnabled(true);
    ui->pushButton_settings->setEnabled(true);

}

void UserLoginDlg::on_loginFinished(int errorCode, int opType)
{
    if(errorCode==0)
    {
        if(TYPE_LOGIN == opType|| TYPE_AUTH_TOKEN == opType)
        {
            if(NULL != m_dlgLoading)
                m_dlgLoading->setText(tr("Getting resource lists ..."));
            callListUserRes();
            //save settings
            if(0 == m_loginSettings.iRemember)
            {
                m_loginSettings.stUserInfo.password[0] = '\0';//
            }
            if(NULL != g_pConfigInfo)
            {
                int iRet = g_pConfigInfo->setSettings_login(m_loginSettings);
                if( iRet < 0)
                {
                    LOG_ERR("save config info failed return value:%d", iRet);
                }
            }
            strcpy(m_loginSettings.stUserInfo.password, ui->lineEdit_password->text().toUtf8().data());
        }
        else if(TYPE_LIST_USER_RES == opType)
        {
            if(NULL != m_dlgLoading)
                m_dlgLoading->setText(tr("Getting user informations..."));
            m_bHasGetListInfo = true;
            callGetUserInfo();
        }
        else if(TYPE_GETUSERINFO == opType)
        {
            if(NULL != m_dlgLoading)
                m_dlgLoading->setText(tr("Initial interface list..."));
            m_bHasGetUserInfo = true;
        }
        else
        {
            LOG_ERR("Unknown optype:%d", opType);
        }
        if(m_bHasGetListInfo&& m_bHasGetUserInfo )
        {
            if(NULL != m_dlgLoading)
                m_dlgLoading->hide();
            if(NULL != g_desktopListDlg)
            {
                LOG_ERR("%s","NULL != g_desktopListDlg going to delete it");
                delete g_desktopListDlg;
            }
            DesktopListDialog *dlg = new DesktopListDialog(m_stAppList, m_vstVirtualDisks,m_stAppBakList, m_role);
            g_desktopListDlg = dlg;
            dlg->show();
            close();
        }
    }
    else
    {
        setLoginProgress(0, tr("Login ..."));
        processErrorCode(errorCode, opType);
    }

}

/*
 * The backend finishes the UI's request, and call this function to notify UI.
 */

void UserLoginDlg::processCallBackData(int errorCode, int dType, void *pRespondData)
{
    switch(dType)
    {
    case TYPE_GETDOMAIN:
    {
        m_domainTaskUuid = TASK_UUID_NULL;
        DOMAIN_DATA *pDomainVector = (DOMAIN_DATA *)pRespondData;
        QStringList domainList;
//        if(m_bIsConnect && errorCode !=0&&!m_bSettingDlg){
//            if(pDomainVector != NULL){
//                delete pDomainVector;
//                pDomainVector = NULL;
//            }
//            Sleep(2000);
//            __initDomainList();
//            break;
//        }
        if(pDomainVector!=NULL)
        {
            vector<string>::size_type i;
            for(i=0; i<pDomainVector->vstrDomainlists.size(); ++i)
            {
                string domain = pDomainVector->vstrDomainlists[i];
                domainList.append(QString(domain.c_str()));
            }
            delete pDomainVector;
        }
        emit updateDomainList(errorCode, dType, domainList);
        cout << "signal updateDomanList emited" << endl;
        break;
    }
    case TYPE_LOGIN:
    case TYPE_AUTH_TOKEN:
    {
        LOGIN_DATA* pLoginData = (LOGIN_DATA*)pRespondData;
        if(NULL != pLoginData)
        {
            m_stNtAccountInfo = pLoginData->stLoginInfo;
            delete pLoginData;
        }
        emit loginFinished(errorCode, dType);
        break;
    }
    case TYPE_LIST_USER_RES:
    {
        LIST_USER_RESOURCE_DATA* pListUserResData = (LIST_USER_RESOURCE_DATA*) pRespondData;
        if(NULL != pListUserResData)
        {
            m_stAppList = pListUserResData->stAppList;
            m_stAppBakList = pListUserResData->stAppBakList;
            delete pListUserResData;
        }
        emit loginFinished(errorCode, dType);
        break;
    }
    case TYPE_GETUSERINFO:
    {
        GET_USER_INFO_DATA* pGetUserInfoData = (GET_USER_INFO_DATA*)pRespondData;
        if(NULL != pGetUserInfoData)
        {
            m_vstVirtualDisks = pGetUserInfoData->vstVirtualDisks;
            m_role = pGetUserInfoData->stNtAccountInfo.Role;
            delete pGetUserInfoData;
        }
        emit loginFinished(errorCode, dType);
        break;
    }
    }
}

void UserLoginDlg::on_combox_domainlist_update(int errorCode, int dType, QStringList domainList)
{
    cout << "SLOT on_combox_domainlist_update called" << endl;
    setGetDomainBusy(false);
    cout << "errorCode: " << errorCode;
    if(errorCode!=0)
        processErrorCode(errorCode, dType);
    else
    {
        ui->comboBox_domain->clear();
        ui->comboBox_domain->addItems(domainList);

        if(g_ipcClient != NULL)
            g_ipcClient->sendWebsocketNetworkInfo(m_vClientSettings.m_network.stPresentServer.serverAddress, m_vClientSettings.m_network.stPresentServer.port);//g_ipcClient->sendWebsocketNetworkInfo(m_vClientSettings.m_network.presentServer, m_vClientSettings.m_network.port);

    //get current item to be selected
        if(m_loginSettings.iRemember)
        {
            if( 0 < strlen(m_loginSettings.stUserInfo.domain))
            {
                int iItermCount = ui->comboBox_domain->count();
                for(int i = 0; i < iItermCount; i++)
                {
                    if(0 == ui->comboBox_domain->itemText(i).compare(m_loginSettings.stUserInfo.domain))
                    {
                        ui->comboBox_domain->setCurrentIndex(i);
                        break;
                    }
                }
            }
        }
        else
        {
            ui->comboBox_domain->setCurrentIndex(0);
        }

        cerr << "m_bAutoLogin: " << m_bAutoLogin << "m_loginSettings.iAutoLogin: " <<  m_loginSettings.iAutoLogin << endl;
        //If auto login, then do this
        if(m_bAutoLogin && m_loginSettings.iAutoLogin)
        {
            on_pushButton_login_clicked();
        }
    }
}

void UserLoginDlg::on_pushButton_login_clicked()
{
    if(NULL == ui)
    {
        return;
    }
    if(0==strlen(m_vClientSettings.m_network.stAlternateServer.serverAddress) &&0==strlen(m_vClientSettings.m_network.stAlternateServer.port)
            && 0==strlen(m_vClientSettings.m_network.stPresentServer.serverAddress) && 0==strlen(m_vClientSettings.m_network.stPresentServer.port))//if(0 == strlen(m_vClientSettings.m_network.firstServer) && 0 ==  strlen(m_vClientSettings.m_network.alternateServer))
    {
        CMessageBox::CriticalBox(tr("Please configure your server address first"));
        return;
    }
    if(0 == ui->tabWidget_login->currentIndex())
    {
//get info in ui
        if (ui->lineEdit_username->text().isEmpty() || ui->lineEdit_password->text().isEmpty())
        {
            CMessageBox::CriticalBox(tr("Information incomplete"));
            return;
        }

        if (ui->lineEdit_username->text().count(' ') == ui->lineEdit_username->text().length()
                || ui->lineEdit_username->text().count('.') == ui->lineEdit_username->text().length())
        {
            CMessageBox::CriticalBox(tr("Username format is invalid, it can not "
                               "contain /\[]\":;|<>+=,?*\nPlease type other username"));
            return;
        }

        if( Qt::Checked  == ui->checkBox_rempwd->checkState())
            m_loginSettings.iRemember = 1;
        else
            m_loginSettings.iRemember = 0;

        if( Qt::Checked == ui->checkBox_autolg->checkState())
            m_loginSettings.iAutoLogin = 1;
        else
            m_loginSettings.iAutoLogin = 0;
        strcpy(m_loginSettings.stUserInfo.username, ui->lineEdit_username->text().toUtf8().data());
        strcpy(m_loginSettings.stUserInfo.password, ui->lineEdit_password->text().toUtf8().data());
        if(0== ui->comboBox_domain->currentText().compare("Local Domain"))
            m_loginSettings.stUserInfo.domain[0] = '\0';
        else
            strcpy(m_loginSettings.stUserInfo.domain, ui->comboBox_domain->currentText().toUtf8().data());        
    }
    else
    {
        if (ui->lineEdit_username_token->text().isEmpty() || ui->lineEdit_password_roken->text().isEmpty())
        {
            CMessageBox::CriticalBox(tr("Information incomplete"));
            return;
        }

        if (ui->lineEdit_username_token->text().count(' ') == ui->lineEdit_username_token->text().length()
                || ui->lineEdit_password_roken->text().count('.') == ui->lineEdit_password_roken->text().length())
        {
            CMessageBox::CriticalBox(tr("Username format is invalid, it can not "
                               "contain /\[]\":;|<>+=,?*\nPlease type other username"));
            return;
        }
    }
    loginSession();
}

int UserLoginDlg::loginSession()
{
    int iRet = 0;

    if(m_isGettingDomain && 0 == ui->tabWidget_login->currentIndex())
    {
        CMessageBox::WarnBox(tr("It is getting domain list , please wait for a while"));
        return 0;
    }

    if(0 != ui->tabWidget_login->currentIndex())
    {//if current login way is authon token ,then stop get domain task
        if(NULL!=m_pSession && TASK_UUID_NULL!=m_domainTaskUuid)
        {
            int iRet = m_pSession->cancelTask(m_domainTaskUuid);
            LOG_INFO("cancel get domain data return value:%d", iRet);
        }
    }

    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return -1;
    }
    setLoginProgress(1, tr("Login ..."));

    pCall_param->pUi = this;
    pCall_param->uiType = LOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;

    if(0 == ui->tabWidget_login->currentIndex())
    {
        m_pSession->setSessionInfo(&(m_vClientSettings.m_network), &(m_loginSettings.stUserInfo));
        iRet = m_pSession->loginSession(param);
    }
    else
    {
        USER_INFO st_tokenInfo;
        strcpy(st_tokenInfo.username, ui->lineEdit_username_token->text().toUtf8().data());
        strcpy(st_tokenInfo.password, ui->lineEdit_password_roken->text().toUtf8().data());
        memset(&(st_tokenInfo.domain), 0, MAX_LEN);
        m_pSession->setSessionInfo(&(m_vClientSettings.m_network), &(st_tokenInfo));
        iRet = m_pSession->authToken(param);
    }
    return iRet;
}

int UserLoginDlg::callListUserRes()
{
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return 0;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = LOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    return m_pSession->listUserResource(param);
}

int UserLoginDlg::callGetUserInfo()
{
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return 0;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = LOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    return m_pSession->getUserInfo(param);
}

int UserLoginDlg::callGetMonitorsInfo()
{
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return 0;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = LOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    return m_pSession->getMonitorsInfo(param);
}

void UserLoginDlg::on_tabWidget_login_currentChanged(int index)
{
    if(index==NORMAL)
        m_eLoginType = NORMAL;
    else
        m_eLoginType = TOKEN;
}

void UserLoginDlg::on_pushButton_about_clicked()
{
    CMessageBox::AboutBox(NULL);
}

void UserLoginDlg::on_settings()
{
    //int iRet = 0;

    //Cancel current getDomain thread if it still running
    if(m_pSession!=NULL)
         m_pSession->cancelTask(m_domainTaskUuid);
    if(m_bIsConnect){
        m_bIsConnect = false;
    }
    setGetDomainBusy(false);

    NetWorkSettingDialog settingWindow(false);
    m_bSettingDlg = true;
    settingWindow.exec();
    m_bSettingDlg = false;
    if(g_bSystemShutDownSignal)
    {//in win7 when updating vClient(using install package), if vClient is not closed. it may crashes the config file
        LOG_ERR("%s","g_bSystemShutDownSignal is true, return directly");
        return;
    }
    DOMAIN_DATA domainData;
    settingWindow.getSettingResult(m_vClientSettings, domainData);
    if(NULL != g_pConfigInfo)
    {
        g_pConfigInfo->setSettings_vclient(m_vClientSettings);
    }
    //int len = domainData.vstrDomainlists.size();
    QStringList domainList;
    for(unsigned int i = 0; i <domainData.vstrDomainlists.size(); i++ )
    {
        string domain = domainData.vstrDomainlists[i];
        domainList.append(QString(domain.c_str()));
    }
    if(domainList.size() > 0)
        on_combox_domainlist_update(0, 0, domainList);
}

void UserLoginDlg::on_checkBox_rempwd_clicked(bool checked)
{
    if(!checked)
        ui->checkBox_autolg->setChecked(false);
}

void UserLoginDlg::on_checkBox_autolg_clicked(bool checked)
{
    if(checked)
        ui->checkBox_rempwd->setChecked(true);
}

void UserLoginDlg::on_pushButton_settings_clicked()
{

}
