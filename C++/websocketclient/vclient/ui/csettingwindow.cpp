#include <QtGui>
#include "csettingwindow.h"
#include "ui_settingwindow.h"
#include "titlewidget.h"
#include "../common/log.h"
#include "../backend/csession.h"
#include "../common/common.h"
#include "../common/cmutexop.h"
#include "../common/cthread.h"
#include "cupdate.h"
#include "cmessagebox.h"
#include <iostream>
#include <QTabBar>
#include "../imageconf.h"

#define ERROR_OK 0
#define ERROR_NOT_CONFIG_SERVER_IP -1110
#define ERROR_PORT  -1111
#define ERROR_PREFERENCE_IP_INVALID -1112
#define ERROR_ALTERNATE_IP_INVALID -1113
#define ERROR_SERVER_IP_SAME -1114

#ifdef  WIN32
    #define     TRACERT_COMMAND     "tracert"
#else
    #define     TRACERT_COMMAND     "traceroute"
#endif

////mutext op//
//////define some global variable
static CMutexOp* gpMutex_getDomain = NULL; //to limit it only useful in this file
static bool gbWindownReleased = false;  //used to identify whethe the setwinodw has closed


int callback_settingWindow(CALLBACK_PARAM_UI*, int, void* );

CSettingWindow::CSettingWindow(const SETTINGS_VCLIENT& stSettings, QWidget *parent) :
    QDialog(parent),
    m_pUi(new Ui::SettingWindow),
    m_isMove(false),
    m_isNetworkSetting(false)
{
    m_titleWidget = NULL;
    m_pSession = NULL;
    m_settingSet_In = stSettings;
    m_settingSet_out = stSettings;
    m_process_execCmd = NULL;
    m_taskUUid = TASK_UUID_NULL;
    m_pUpdateVClientThread = NULL;
    //m_bUpdateDomainSucceed = false;

    memset(&m_userinfo, 0, sizeof(USER_INFO));
    m_pMutex_getDomain = new CMutexOp();
    gbWindownReleased = false;
    if(NULL != gpMutex_getDomain)
    {
        delete gpMutex_getDomain;
        gpMutex_getDomain = NULL;
    }

    //VCLIENT_MUTEX_INIT(mutex_getDomain)
    initialize();
}


int CSettingWindow::initialize()
{
//initialize UI
    m_pUi->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint|Qt::WindowMinimizeButtonHint);
    //setWindowIcon(QIcon(QPixmap(ICON_PATH)));
    QPalette pal = palette();
    pal.setBrush(QPalette::Window, QBrush(QPixmap(IMAGE_PATH_SETTING_WINDOWS_BACKGROUND)));
    setPalette(pal);
//    QString styleSheet = "QDialog{"
//            "background-image:url(\"";
//    styleSheet = styleSheet + IMAGE_PATH_SETTING_WINDOWS_BACKGROUND + "\");";
//    styleSheet = styleSheet + "border:1px solid #e9e9e9;"
//            "border-radius:6px;"
//            "margin:3px;"
//            "}";
//    setStyleSheet(styleSheet);
    //setWindowIcon(QIcon(WINDOWS_ICON));
    setWindowIcon(QIcon(WINDOWS_IMG.data()));
    setDialogInCenter(this);

//    QLabel *titleLabel= new QLabel(tr("<strong><font color=#77b200>&nbsp;Settings</font></strong>"), this);
//    titleLabel->move(10,4);
//    m_titleWidget = new TitleWidget(NOMAXMIZE, "", this);

    m_pUi->tabWidget->setCurrentIndex(0);
	m_pUi->tabWidget->setFocusPolicy(Qt::NoFocus);

#ifdef Q_WS_X11
    m_pUi->tabWidget->removeTab(1);
#endif

//connecting slot
    connect(m_pUi->pushBtn_ok, SIGNAL(clicked()), this, SLOT(on_btnOkPressed()));
    connect(m_pUi->pushBtn_cancel, SIGNAL(clicked()), this, SLOT(on_btnCancelPressed()));
    connect(m_pUi->pushBtn_apply, SIGNAL(clicked()), this, SLOT(on_btnApplyPressed()));
    connect(m_pUi->pushBtn_execute, SIGNAL(clicked()), this, SLOT(on_btnExecutePressed()));
    connect(m_pUi->pushBtn_check, SIGNAL(clicked()), this, SLOT(on_btnCheckUpdatePressed()));
    connect(this, SIGNAL(getDomainFinished(int,DOMAIN_DATA)),
            this, SLOT(on_updateDomainFinished(int,DOMAIN_DATA)));

    connect(m_pUi->lineEdit_alternateIp, SIGNAL(textEdited(QString)), this, SLOT(on_ChangedInWidget(QString)));
//    connect(m_pUi->lineEdit_port, SIGNAL(textEdited(QString)), this, SLOT(on_ChangedInWidget(QString)));
    connect(m_pUi->lineEdit_preferenceIp, SIGNAL(textEdited(QString)), this, SLOT(on_ChangedInWidget(QString)));
//    connect(m_pUi->comboBox_protocol, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_ChangedInWidget(QString)));
    connect(m_pUi->radioBtn_fap_hasbar, SIGNAL(toggled(bool)), this, SLOT(on_ChangedInWidget(bool)));
    connect(m_pUi->radioBtn_rdp_hasbar, SIGNAL(toggled(bool)), this, SLOT(on_ChangedInWidget(bool)));
    connect(m_pUi->radioBtn_update_auto, SIGNAL(toggled(bool)), this, SLOT(on_ChangedInWidget(bool)));
    connect(m_pUi->radioBtn_update_auto2, SIGNAL(toggled(bool)), this, SLOT(on_ChangedInWidget(bool)));
    connect(m_pUi->checkBox_lounchAppOnSysStart, SIGNAL(clicked(bool)), this, SLOT(on_ChangedInWidget(bool)));
    connect(this, SIGNAL(getKey_enterPressed(int)), this, SLOT(on_getKey_enterPressed(int)));
    SetPortValidator(m_pUi->lineEdit_port);
    SetUrlValidator(m_pUi->lineEdit_preferenceIp);
    SetUrlValidator(m_pUi->lineEdit_alternateIp);

    loadSettings();
   // m_pUi->radioBtn_ping->setChecked(true);
    //m_pUi->pushBtn_apply->setEnabled(false);

    //combobox_cmd
    QString str_cmd_ping = "ping";
    QString str_cmd_nslookup = "nslookup";
    QString str_cmd_tracert = TRACERT_COMMAND;

    m_pUi->comboBoxCmd->addItem(str_cmd_ping);
    m_pUi->comboBoxCmd->addItem(str_cmd_nslookup);
    m_pUi->comboBoxCmd->addItem(str_cmd_tracert);

    return ERROR_OK;
}

int CSettingWindow::loadSettings()
{
    if(NULL == m_pUi)
    {
        return -1;
    }
    m_pUi->lineEdit_preferenceIp->setText(m_settingSet_In.m_network.stFirstServer.serverAddress);
    m_pUi->lineEdit_alternateIp->setText(m_settingSet_In.m_network.stAlternateServer.serverAddress);
    m_pUi->comboBox_protocol->setCurrentIndex(m_settingSet_In.m_network.stFirstServer.isHttps == 0? 0 : 1);
//    m_pUi->lineEdit_preferenceIp->setText(m_settingSet_In.m_network.firstServer);
//    m_pUi->lineEdit_alternateIp->setText(m_settingSet_In.m_network.alternateServer);
//    m_pUi->comboBox_protocol->setCurrentIndex(m_settingSet_In.m_network.isHttps == 0? 0 : 1);

    if(CUpdate::getAutoUpdateServiceStatus()>0)
    {
        m_pUi->radioBtn_update_auto->setChecked(false);
        m_pUi->radioBtn_update_manual->setChecked(false);
        m_pUi->radioBtn_update_auto2->setChecked(true);
        m_settingSet_In.m_updateSetting = AUTO_UPDATE;
    }
    else
    {
        if(AUTO_DETECT == m_settingSet_In.m_updateSetting )
        {
            m_pUi->radioBtn_update_auto->setChecked(true);
            m_pUi->radioBtn_update_manual->setChecked(false);
            m_pUi->radioBtn_update_auto2->setChecked(false);
        }
        else
        {
            m_settingSet_In.m_updateSetting = MANUAL_UPDATE;
            m_pUi->radioBtn_update_auto->setChecked(false);
            m_pUi->radioBtn_update_manual->setChecked(true);
            m_pUi->radioBtn_update_auto2->setChecked(false);
        }
    }

//    if(AUTO_DETECT == m_settingSet_In.m_updateSetting )
//    {
//        m_pUi->radioBtn_update_auto->setChecked(true);
//        m_pUi->radioBtn_update_manual->setChecked(false);
//        m_pUi->radioBtn_update_auto2->setChecked(false);
//    }
//    else if(AUTO_UPDATE == m_settingSet_In.m_updateSetting)
//    {
//        if(CUpdate::getAutoUpdateServiceStatus()>0)
//        {
//            m_pUi->radioBtn_update_auto->setChecked(false);
//            m_pUi->radioBtn_update_manual->setChecked(false);
//            m_pUi->radioBtn_update_auto2->setChecked(true);
//        }
//        else
//        {
//            m_pUi->radioBtn_update_auto->setChecked(false);
//            m_pUi->radioBtn_update_manual->setChecked(true);
//            m_pUi->radioBtn_update_auto2->setChecked(false);
//        }
//    }
//    else
//    {
//        m_pUi->radioBtn_update_auto->setChecked(false);
//        m_pUi->radioBtn_update_manual->setChecked(true);
//        m_pUi->radioBtn_update_auto2->setChecked(false);
//    }

    if(NOBAR_STATE == m_settingSet_In.m_rdpBar)
    {
        m_pUi->radioBtn_rdp_hasbar->setChecked(false);
        m_pUi->radioBtn_rdp_nobar->setChecked(true);
    }
    else
    {
        m_pUi->radioBtn_rdp_hasbar->setChecked(true);
        m_pUi->radioBtn_rdp_nobar->setChecked(false);
    }

    if(NOBAR_STATE == m_settingSet_In.m_fapBar)
    {
        m_pUi->radioBtn_fap_hasbar->setChecked(false);
        m_pUi->radioBtn_fap_nobar->setChecked(true);

    }
    else
    {
        m_pUi->radioBtn_fap_hasbar->setChecked(true);
        m_pUi->radioBtn_fap_nobar->setChecked(false);
    }
#ifdef WIN32
    if(LOUNCH_ON_START == getLaunchOnSysStart())
    {
        m_pUi->checkBox_lounchAppOnSysStart->setChecked(true);
    }
    else
    {
        m_pUi->checkBox_lounchAppOnSysStart->setChecked(false);
    }
#else
    m_pUi->groupBox_2->hide();
#endif

    return 0;
}

CSettingWindow::~CSettingWindow()
{
    if(NULL != m_pMutex_getDomain)
    {
        m_pMutex_getDomain->lock();
        gbWindownReleased = true;
        m_pMutex_getDomain->unlock();
    }
    else
    {
        gbWindownReleased = true;
    }

    if(NULL != m_pSession)
    {
        //delete m_pSession;
        if(TASK_UUID_NULL != m_taskUUid)
            m_pSession->cancelTask(m_taskUUid);
        CThread::createThread(NULL, NULL, (FUN_THREAD)(&releaseCSession), (void*)m_pSession);
        m_pSession = NULL;
    }
    if(NULL != m_titleWidget)
    {
        delete m_titleWidget;
        m_titleWidget = NULL;
    }
    if(NULL != m_pUi)
    {
        delete m_pUi;
        m_pUi = NULL;
    }
    if(NULL != m_pMutex_getDomain)
    {
        delete m_pMutex_getDomain;
        m_pMutex_getDomain = NULL;
    }
    if( NULL != m_process_execCmd)
    {
        if(m_process_execCmd->state() == QProcess::Running)
            m_process_execCmd->kill();
        delete m_process_execCmd;
        m_process_execCmd = NULL;
    }
    if(NULL != m_pUpdateVClientThread)
    {
        disconnect(m_pUpdateVClientThread, SIGNAL(finished()), this, SLOT(on_UpdateThreadFinished()));
        delete m_pUpdateVClientThread;
        m_pUpdateVClientThread = NULL;
    }
}
int CSettingWindow::checkValidation(bool b_ShowMsgBox/* = false */)
{
    if(m_pUi->lineEdit_preferenceIp->text().isEmpty() && m_pUi->lineEdit_alternateIp->text().isEmpty())
    {
        if(b_ShowMsgBox)
            CMessageBox::CriticalBox(tr("Information incomplete"), NULL);
        return ERROR_NOT_CONFIG_SERVER_IP;
    }
    if(0 == m_pUi->lineEdit_preferenceIp->text().compare(m_pUi->lineEdit_alternateIp->text()))
    {
        if(b_ShowMsgBox)
            CMessageBox::WarnBox(tr("preference and alternative vAccess IP is the same"), NULL);
        return ERROR_SERVER_IP_SAME;
    }

    if (!IsValidPort(m_pUi->lineEdit_port->text()))
    {
        if(b_ShowMsgBox)
            CMessageBox::CriticalBox(tr("Port format incorrect"), NULL);
        return ERROR_PORT;
    }

    if(!m_pUi->lineEdit_preferenceIp->text().isEmpty() && !IsValidUrl(m_pUi->lineEdit_preferenceIp->text()))
    {
        if(b_ShowMsgBox)
            CMessageBox::CriticalBox(tr("IP format incorrect"), NULL);
        return ERROR_PREFERENCE_IP_INVALID;
    }

    if(!m_pUi->lineEdit_alternateIp->text().isEmpty())
    {
        if(!IsValidUrl(m_pUi->lineEdit_alternateIp->text()))
        {
            if(b_ShowMsgBox)
                CMessageBox::CriticalBox(tr("IP format incorrect"), this);
            return ERROR_ALTERNATE_IP_INVALID;
        }
    }
    return ERROR_OK;
}

int CSettingWindow::getSettings()
{// please make sure that all the settings are valid(call checkValidation to verify)
//write network settings
    strcpy(m_settingSet_out.m_network.stFirstServer.port, m_pUi->lineEdit_port->text().toUtf8().data());
    strcpy(m_settingSet_out.m_network.stAlternateServer.port, m_pUi->lineEdit_port->text().toUtf8().data());
    strcpy(m_settingSet_out.m_network.stFirstServer.serverAddress, m_pUi->lineEdit_preferenceIp->text().toUtf8().data());
    strcpy(m_settingSet_out.m_network.stAlternateServer.serverAddress, m_pUi->lineEdit_alternateIp->text().toUtf8().data());
//    strcpy(m_settingSet_out.m_network.port, m_pUi->lineEdit_port->text().toUtf8().data());
//    strcpy(m_settingSet_out.m_network.firstServer, m_pUi->lineEdit_preferenceIp->text().toUtf8().data());
//    strcpy(m_settingSet_out.m_network.alternateServer, m_pUi->lineEdit_alternateIp->text().toUtf8().data());
    if( 0 == m_pUi->comboBox_protocol->currentIndex())
    {
        m_settingSet_out.m_network.stFirstServer.isHttps = 0;//m_settingSet_out.m_network.isHttps = 0;
        m_settingSet_out.m_network.stAlternateServer.isHttps = 0;
    }
    else
    {
        m_settingSet_out.m_network.stFirstServer.isHttps = 1;//m_settingSet_out.m_network.isHttps = 1;
        m_settingSet_out.m_network.stAlternateServer.isHttps = 1;
    }
//write auto update
    if(m_pUi->radioBtn_update_auto->isChecked())
    {
        m_settingSet_out.m_updateSetting = AUTO_DETECT;
    }
    else if (m_pUi->radioBtn_update_auto2->isChecked())
    {
        m_settingSet_out.m_updateSetting = AUTO_UPDATE;
    }
    else
    {
        m_settingSet_out.m_updateSetting = MANUAL_UPDATE;
    }
//write fap has bar
    if(m_pUi->radioBtn_fap_hasbar->isChecked())
    {
        m_settingSet_out.m_fapBar = HASBAR_STATE;
    }
    else
    {
        m_settingSet_out.m_fapBar = NOBAR_STATE;
    }
//write rdp has bar
    if(m_pUi->radioBtn_rdp_hasbar->isChecked())
    {
        m_settingSet_out.m_rdpBar = HASBAR_STATE;
    }
    else
    {
        m_settingSet_out.m_rdpBar = NOBAR_STATE;
    }
//write lounch on system start
    if(m_pUi->checkBox_lounchAppOnSysStart->isChecked())
    {
        m_settingSet_out.m_louchAppOnSysStart = LOUNCH_ON_START;
    }
    else
    {
        m_settingSet_out.m_louchAppOnSysStart = NOT_LOUNCH_ON_START;
    }
    return ERROR_OK;
}

int CSettingWindow::setWidgetEnabled(bool bEnabled)
{
    if(NULL != m_pUi)
    {
        m_pUi->lineEdit_alternateIp->setEnabled(bEnabled);
        m_pUi->lineEdit_port->setEnabled(bEnabled);
        m_pUi->lineEdit_preferenceIp->setEnabled(bEnabled);
        m_pUi->comboBox_protocol->setEditable(bEnabled);
        m_pUi->pushBtn_apply->setEnabled(bEnabled);
        m_pUi->pushBtn_ok->setEnabled(bEnabled);
    }
    return 0;
}

int CSettingWindow::dealSettings()
{
    int iRet = checkValidation(true);
    if(iRet < 0)
    {
        if(iRet == ERROR_NOT_CONFIG_SERVER_IP || iRet == ERROR_PORT || iRet == ERROR_SERVER_IP_SAME
           || iRet == ERROR_PREFERENCE_IP_INVALID || iRet == ERROR_ALTERNATE_IP_INVALID)
        {
            if( NULL != m_pUi)
                m_pUi->tabWidget->setCurrentIndex(0);
        }
        return -1;
    }

    iRet = getSettings();
    if(iRet < 0)
    {
        return iRet;
    }
//    if(NULL != g_pConfigInfo)
//    {
//        g_pConfigInfo->setSettings_vclient(m_settingSet_out);
//    }
    m_pUi->pushBtn_apply->setEnabled(false);
    m_pUi->pushBtn_cancel->setEnabled(true);
    m_pUi->pushBtn_ok->setEnabled(false);
    m_pUi->tab_network->setEnabled(false);

    iRet = setLaunchOnSysStart(m_settingSet_out.m_louchAppOnSysStart);
    if(iRet < 0)
    {
        m_pUi->pushBtn_cancel->setEnabled(true);
        m_pUi->pushBtn_ok->setEnabled(true);
        m_pUi->tab_network->setEnabled(true);
        return iRet;
    }
    m_pUi->pushBtn_apply->setEnabled(false);
    if( NULL != m_pSession )
    {
        //delete m_pSession;
        if(TASK_UUID_NULL != m_taskUUid)
            m_pSession->cancelTask(m_taskUUid);
        CThread::createThread(NULL, NULL, (FUN_THREAD)(&releaseCSession), (void*)m_pSession);
        m_pSession = NULL;
    }

    bool bAutoUpdate = m_pUi->radioBtn_update_auto2->isChecked();
    iRet = CUpdate::setAutoUpdate(bAutoUpdate);
    if(iRet < 0)
    {
        CMessageBox::CriticalBox(tr("Set update type failed!"), NULL);
        return iRet;
    }

    if(m_b_isApplyClick && !m_isNetworkSetting)
    {
        m_b_isApplyClick = false;
        m_pUi->pushBtn_cancel->setEnabled(true);
        m_pUi->pushBtn_ok->setEnabled(true);
        m_pUi->tab_network->setEnabled(true);
        return ERROR_OK;
    }
    m_pSession = new CSession(m_settingSet_out.m_network, m_userinfo);
    if( NULL == m_pSession )
    {
        return -1;
    }
    CALLBACK_PARAM_UI *cal_param = new CALLBACK_PARAM_UI;
    cal_param->pUi = this;
    PARAM_SESSION_IN param;
    param.callbackFun = callback_settingWindow;
    param.callback_param = cal_param;
    param.isBlock = UNBLOCK;

    if(NULL == gpMutex_getDomain)
    {
        gpMutex_getDomain = new CMutexOp(*(getMutex()));
    }
    iRet = m_pSession->getDomainList(param);
    if( iRet < 0)
    {
        return iRet;
    }
    m_taskUUid = param.taskUuid;
    return ERROR_OK;
}

int CSettingWindow::on_btnOkPressed()
{
    m_b_isApplyClick = false;
    return dealSettings();
}

int CSettingWindow::on_btnCancelPressed()
{
    m_settingSet_out.m_network = m_settingSet_In.m_network;
    close();
    return 0;
}

int CSettingWindow::on_btnApplyPressed()
{
    m_b_isApplyClick = true;
    return dealSettings();
}

int CSettingWindow::on_btnExecutePressed()
{
    if (m_process_execCmd != NULL)
    {
        if(m_process_execCmd->state() == QProcess::Running)
            m_process_execCmd->kill();
        delete m_process_execCmd;
        m_process_execCmd = NULL;
    }
    QString str;
//    if(m_pUi->radioBtn_ping->isChecked())
//        str = "ping ";
//    else if(m_pUi->radioBtn_traceroute->isChecked())
//        str = TRACERT_COMMAND;
//    else
//        str = "nslookup ";
    str = m_pUi->comboBoxCmd->currentText();
    str += " ";
    str.append( m_pUi->lineEdit_netparam->text());
    m_process_execCmd = new QProcess;
    connect(m_process_execCmd,SIGNAL(readyRead()),this,SLOT(on_readOutput()));
    m_process_execCmd->start(str);
    m_str_cmdOutput.clear();
    return 0;
}

void CSettingWindow::on_readOutput()
{
    m_str_cmdOutput += QString::fromLocal8Bit(m_process_execCmd->readAll());
    if(NULL != m_pUi)
    {
        m_pUi->textEdit_netcmdOutput->setText(m_str_cmdOutput);
        m_pUi->textEdit_netcmdOutput->moveCursor(QTextCursor::End);
    }
}

int CSettingWindow::on_btnCheckUpdatePressed()
{
    int iRet = checkValidation(true);
    if(iRet < 0)
    {
        if(iRet == ERROR_NOT_CONFIG_SERVER_IP || iRet == ERROR_PORT || iRet == ERROR_SERVER_IP_SAME
           || iRet == ERROR_PREFERENCE_IP_INVALID || iRet == ERROR_ALTERNATE_IP_INVALID)
        {
            if( NULL != m_pUi)
                m_pUi->tabWidget->setCurrentIndex(0);
        }
        return -1;
    }

    iRet = getSettings();
    if(iRet < 0)
    {
        return iRet;
    }

    SETTINGS_VCLIENT vclientSettings = m_settingSet_out;
    vclientSettings.m_updateSetting = MANUAL_UPDATE;
    m_pUi->pushBtn_check->setEnabled(false);
    if(NULL != m_pUpdateVClientThread)
    {
        delete m_pUpdateVClientThread;
        m_pUpdateVClientThread = NULL;
    }
    m_pUpdateVClientThread = new CUpdate(vclientSettings);
    connect(m_pUpdateVClientThread, SIGNAL(finished()), this, SLOT(on_UpdateThreadFinished()));
    m_pUpdateVClientThread->start();

//    iRet = runUpdate(&vclientSettings);
//    m_pUi->pushBtn_check->setEnabled(true);
//    if(iRet < 0)
//    {
//        LOG_ERR("runUpdate failed. return value:%d", iRet);
//        CMessageBox::CriticalBox(tr("Run update check process failed"), NULL);
//        return iRet;
//    }
    return 0;
}

int CSettingWindow::on_UpdateThreadFinished()
{
    if(NULL != m_pUpdateVClientThread)
    {
        delete m_pUpdateVClientThread;
        m_pUpdateVClientThread = NULL;
    }
    m_pUi->pushBtn_check->setEnabled(true);
    return 0;
}

int CSettingWindow::on_ChangedInWidget(QString str)
{
    QLineEdit *sender = qobject_cast<QLineEdit *>(this->sender());
    if(sender != NULL)
    {
        if(sender == m_pUi->lineEdit_preferenceIp || sender==m_pUi->lineEdit_alternateIp)
            m_isNetworkSetting = true;
    }
    m_pUi->pushBtn_apply->setEnabled(true);
    m_pUi->pushBtn_cancel->setEnabled(true);
    m_pUi->pushBtn_ok->setEnabled(true);
    return 0;
}

int CSettingWindow::on_ChangedInWidget(bool b)
{
    on_ChangedInWidget(QString(""));
    return 0;
}

void CSettingWindow::closeEvent(QCloseEvent *event)
{
//    if (updateDomainThread != NULL && updateDomainThread->isRunning())
//    {
//        updateDomainThread->terminate();
//        delete updateDomainThread;
//        updateDomainThread = NULL;
////		updateDomainThread->wait();
//    }
    if(NULL != m_pMutex_getDomain)
    {
        m_pMutex_getDomain->lock();
        gbWindownReleased = true;
        m_pMutex_getDomain->unlock();
    }
    else
    {
        gbWindownReleased = true;
    }
    QDialog::closeEvent(event);
}

void CSettingWindow::paintEvent(QPaintEvent *)
{
//    QPainter painter(this);
//    QColor color(0x77, 0xb2, 0x00);//77b200
//    painter.fillRect(0,0, 10, 25, color);//x y, width, height
}

void CSettingWindow::mouseMoveEvent(QMouseEvent *_qm)
{
    if(_qm->buttons() == Qt::LeftButton && m_isMove)
        move(_qm->globalPos() - m_pressPoint);
}

void CSettingWindow::mousePressEvent(QMouseEvent *_qm)
{
    m_isMove = true;
    m_pressPoint = _qm->pos();
}

void CSettingWindow::mouseReleaseEvent(QMouseEvent *)
{
    m_isMove = false;
}

void CSettingWindow::keyPressEvent (QKeyEvent * _qk)
{
    /* modified by hansong 7-11 --start */
     /*Key_Return == Enter-2  */
     /*Key_Enter  == Enter-1  */
    if(_qk->key()==Qt::Key_Return || _qk->key() == Qt::Key_Enter )
    {
    /* modified by hansong 7-11 --end  */
        if(focusWidget() == m_pUi->lineEdit_netparam)
        {
            emit getKey_enterPressed(1);
        }
        else
        {
            emit getKey_enterPressed(0);//fix bug #4296
        }
    }
}

int CSettingWindow::on_getKey_enterPressed(int type)
{
    if(1==type)
    {
        on_btnExecutePressed();
    }
    else
    {
        on_btnOkPressed();
    }
    return 0;
}

int CSettingWindow::on_updateDomainFinished(int state, const DOMAIN_DATA stDomainData)
{
//    setWidgetEnabled(true);
    //state < 0 ? m_bUpdateDomainSucceed = false : m_bUpdateDomainSucceed = true;
    state < 0 ? m_settingSet_out.m_network = m_settingSet_In.m_network : m_settingSet_In.m_network = m_settingSet_out.m_network;
    if (state < 0)
    {
        CMessageBox::CriticalBox(tr("Connect to Server failed"), NULL);//Update domain failure
        m_pUi->pushBtn_ok->setEnabled(true);
        m_pUi->pushBtn_cancel->setEnabled(true);
        m_pUi->tab_network->setEnabled(true);
        m_pUi->pushBtn_apply->setEnabled(false);
        m_b_isApplyClick = false;
        return -1;
        //CMessageBox::CriticalBox(tr("Connect fail"), NULL);
    }
    else if(!m_b_isApplyClick)
    {
        m_domain_data.vstrDomainlists = stDomainData.vstrDomainlists;
        close();
    }
    else
    {
        if(m_isNetworkSetting)
            CMessageBox::WarnBox(tr("Update domain success"), NULL);
        m_pUi->pushBtn_ok->setEnabled(true);
        m_pUi->pushBtn_cancel->setEnabled(true);
        m_pUi->tab_network->setEnabled(true);
        m_b_isApplyClick = false;
        m_isNetworkSetting = false;
        m_domain_data.vstrDomainlists = stDomainData.vstrDomainlists;
        m_pUi->pushBtn_apply->setEnabled(false);
    }
    m_settingSet_out.m_network = m_pSession->getNetwork();
    return 0;
}

int callback_settingWindow(CALLBACK_PARAM_UI* pParamUi, int, void* pVoid)
{
    if( NULL == pVoid || NULL == pParamUi )
    {
        return -1;
    }
    CSettingWindow* pSettingWindow = (CSettingWindow*)pParamUi->pUi;
    DOMAIN_DATA* pDomain = (DOMAIN_DATA*)pVoid;
    if(NULL != gpMutex_getDomain)
    {
        gpMutex_getDomain->lock();
        pSettingWindow->m_taskUUid = TASK_UUID_NULL;
        if(!gbWindownReleased)
        {
            pSettingWindow->emitUpdateDomainFinishedSignal(pParamUi->errorCode, *pDomain);
        }
        gpMutex_getDomain->unlock();
        delete gpMutex_getDomain;
        gpMutex_getDomain = NULL;
    }
    //pSettingWindow->on_updateDomainFinished(pParamUi->errorCode, *pDomain);
    delete(pDomain);
    pDomain = NULL;

    return ERROR_OK;
}

int CSettingWindow::setLaunchOnSysStart(LOUCH_APP_ON_SYS_START isStart)
{
    int i_opType = 0, iRet = 0;
    if(LOUNCH_ON_START != isStart)
    {
        i_opType = 1;
    }
    iRet = lounchOnSystemStart(NULL, 0, i_opType);
    //query
    iRet = lounchOnSystemStart(NULL, 0, 2);
    if(iRet < 0)
    {//current state :not start on lounch
        if(isStart == LOUNCH_ON_START)
        {
            CMessageBox::CriticalBox(tr("Set Louch-On-System-Start Info failed"), NULL);
            m_pUi->checkBox_lounchAppOnSysStart->setCheckState(Qt::Checked);
            return -1;
        }
        else
        {
            m_pUi->checkBox_lounchAppOnSysStart->setCheckState(Qt::Unchecked);
            return 0;
        }
    }
    else
    {//current state:start on lounch
        if(isStart == LOUNCH_ON_START)
        {
            m_pUi->checkBox_lounchAppOnSysStart->setCheckState(Qt::Checked);
            return 0;
        }
        else
        {
            CMessageBox::CriticalBox(tr("Set Louch-On-System-Start Info failed"), NULL);
            m_pUi->checkBox_lounchAppOnSysStart->setCheckState(Qt::Unchecked);
            return -1;
        }
    }

    return 0;
}

LOUCH_APP_ON_SYS_START CSettingWindow::getLaunchOnSysStart()
{
    int iRet = lounchOnSystemStart(NULL, 0, 2);
    if(iRet >= 0)
    {
        return LOUNCH_ON_START;
    }
    else
        return NOT_LOUNCH_ON_START;
}

void CSettingWindow::on_comboBox_protocol_currentIndexChanged(int index)
{
    if(index == 0)
        m_pUi->lineEdit_port->setText("80");
    else
        m_pUi->lineEdit_port->setText("443");
    m_isNetworkSetting = true;
    on_ChangedInWidget(QString(""));
}
