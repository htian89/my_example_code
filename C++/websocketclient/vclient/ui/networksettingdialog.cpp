#include "networksettingdialog.h"
#include "ui_networksettingdialog.h"
#include "../config.h"
#include "titlewidget.h"
#include "cmessagebox.h"
#include "../common/common.h"
#include "../common/log.h"
#include "../common/cconfiginfo.h"
#include "../backend/csession.h"
#include "userlogindlg.h"
#include "cupdate.h"
#include "multiAccessesDialog.h"
#include "../common/MultiAccesses.h"
#include <QMouseEvent>
#include <QProcess>
#include <QStringList>
#include "../common/rh_ifcfg.h"
#include "../imageconf.h"

// debug
#include <iostream>
using namespace std;

extern CConfigInfo* g_pConfigInfo; //defined in main.cpp
extern bool g_bSystemShutDownSignal;    //From desktoplistdialog.cpp
extern IMAGE_S vclient_image;
QProcess *process_cmd = NULL;

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
#define     PING_COMMAND  "ping -c 10 "
#endif
////mutext op//
//////define some global variable
static CMutexOp* gpMutex_getDomain_networkSetDlg = NULL; //to limit it only useful in this file
static bool gbWindowReleased_networkSetDlg = false;  //used to identify whethe the setwinodw has closed

NetWorkSettingDialog::NetWorkSettingDialog(bool bShowLoginWindow/* = true*/, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetWorkSettingDialog),
    m_isMove(false),
    m_isUiEnable(true)
{
    m_bShowLoginWindow = bShowLoginWindow;
    //m_process_execCmd = NULL;
    m_pSession = NULL;
    m_taskUuid = TASK_UUID_NULL;
    m_pUpdateVClientThread = NULL;

    ui->setupUi(this);
    ui->comboBox_cmd->setItemText(1, TRACERT_COMMAND);
    m_pMovie = new QMovie(IMAGE_LOADING);
    m_pMovie->setScaledSize(QSize(200, 15));
    ui->label_loading->setMovie(m_pMovie);
    //ui->label_title->setPixmap(QPixmap(CONFIG_TITLE_IMG));
    LOG_INFO("vclient_image.Fronview_configure_title is %s", vclient_image.Fronview_configure_title.c_str());
    ui->label_title->setPixmap(QPixmap(vclient_image.Fronview_configure_title.c_str()));

    ui->tabWidget->setCurrentIndex(0);
    ui->pushButton_cancel->hide();
    ui->pushButton_close_netCmd->hide();

#ifdef _WIN32
    ui->comboBox_preferenceIp->hide();
    ui->pushButton_multiAccess->hide();
#endif

#ifndef _WIN32
    ui->comboBox_preferenceIp->setLineEdit(ui->lineEdit_preferenceIp);
    ui->comboBox_preferenceIp->setEditable(true);
#endif


    refreshAccessIpList();



    QString string ="QTabBar::tab{"
            "font: bold 15px;"
            "color: rgb(0, 0, 0);"
            "border: 0px solid #C4C4C3;"
            "border-bottom-color: #C2C7CB;"
            "border-top-left-radius: 0px;"
            "border-top-right-radius: 0px;"
            "min-width: 80px;"
            "margin-left:3px;"
            "min-height: 18px;"
            "padding: 5px;}"
            "QTabBar::tab:selected{"
            "font: bold 15px;"
            "border-color: #9B9B9B;"
            "border-top-left-radius:2px;"
            "border-top-right-radius:2px;"
            "border-bottom-color: #C2C7CB;"
            "background-color: #e9e9e9;}"
            "QTabBar::tab:!selected{"
            "font: bold 15px;"
            "border-radius:4px;"
            "margin-top: 0px;}"
            "QTabBar::tab:first{"
            "margin-left: 20px;"
            "}";
    ui->tabWidget->getTabBar()->setStyleSheet(string);
    ui->comboBox_cmd->setStyleSheet(STYLE_SHEET_COMBO_BOX_OTHER);
    ui->comboBox_protocol->setStyleSheet(STYLE_SHEET_COMBO_BOX_OTHER);
    ui->comboBox_protocol_2->setStyleSheet(STYLE_SHEET_COMBO_BOX_OTHER);
    ////hide the 3rd tab
#ifdef Q_WS_X11
    ui->tabWidget->removeTab(2);
#endif

    setStyleSheet(STYLE_SHEET_PUSHBTN);

    m_pMutex_getDomain = new CMutexOp();
    gbWindowReleased_networkSetDlg = false;
    if(NULL != gpMutex_getDomain_networkSetDlg)
    {
        delete gpMutex_getDomain_networkSetDlg;
        gpMutex_getDomain_networkSetDlg = NULL;
    }
//    setAttribute(Qt::WA_TranslucentBackground);
    m_backgroundPixmap.fill(QColor(0xe9, 0xe9, 0xe9));
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    //setWindowIcon(QIcon(WINDOWS_ICON));
    setWindowIcon(QIcon(WINDOWS_IMG.data()));
    //    QPalette pal = palette();
    //    pal.setBrush(QPalette::Window, QBrush(QPixmap(IMAGE_PATH_SETTING_WINDOWS_BACKGROUND)));
    //    setPalette(pal);


    m_titleWidget = new TitleWidget(NOMAXMIZE, "", this);

    m_titleWidget->setImage(vclient_image.icon_close.c_str(), CLOSE);
    m_titleWidget->setImage(vclient_image.icon_minimize.c_str(), MINIMIZE);

    m_titleWidget->setLayout_title(0, 0, 10, 0);
    setWindowTitle(tr("Settings"));

    //connect slot
    connect(ui->pushButton_execute, SIGNAL(clicked()), this, SLOT(on_pushBtn_executePressed()));
    connect(ui->radioButton_update_auto, SIGNAL(toggled(bool)), this, SLOT(on_radioBtn_updateType_changed(bool)));
    connect(ui->radioButton_update_auto2, SIGNAL(toggled(bool)), this, SLOT(on_radioBtn_updateType_changed(bool)));
    connect(ui->checkBox_lounchAppOnSysStart, SIGNAL(clicked(bool)), this, SLOT(on_checkBox_start_type_changed(bool)));
    connect(ui->pushButton_apply_other_settings, SIGNAL(clicked()), this, SLOT(on_pushBtn_apply_other_settings()));
    connect(ui->comboBox_protocol, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_protocol_currentIndexChanged(int)));
    connect(ui->comboBox_protocol_2, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_protocol_2_currentIndexChanged(int)));
    //connect(ui->pushBtn_connect, SIGNAL(clicked()), this, SLOT(on_pushBtn_connect_clicked()));
    connect(this, SIGNAL(getDomainFinished(int,DOMAIN_DATA)), this, SLOT(on_updateDomainFinished(int,DOMAIN_DATA)));

    connect(ui->lineEdit_preferenceIp, SIGNAL(textChanged(QString)), this, SLOT(on_Changed_inNetworkSetting(QString)));
    connect(ui->lineEdit_port, SIGNAL(textChanged(QString)), this, SLOT(on_Changed_inNetworkSetting(QString)));
    connect(ui->comboBox_protocol, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_Changed_inNetworkSetting(QString)));
    connect(ui->lineEdit_alternateIp, SIGNAL(textChanged(QString)), this, SLOT(on_Changed_inNetworkSetting(QString)));
    connect(ui->lineEdit_port_2, SIGNAL(textChanged(QString)), this, SLOT(on_Changed_inNetworkSetting(QString)));
    connect(ui->comboBox_protocol_2, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_Changed_inNetworkSetting(QString)));
    connect(ui->checkBox_autoConnect, SIGNAL(toggled(bool)), this, SLOT(on_Changed_inNetworkSetting(bool)));

    // connect(ui->pushButton_close_netCmd, SIGNAL(clicked()), this, SLOT(on_pushBtn_close_netCmd()));
    connect(ui->pushButton_updateCheck, SIGNAL(clicked()), this, SLOT(on_pushBtn_updateCheckClicked()));
    connect(this, SIGNAL(getKey_enterPressed(int)), this, SLOT(on_getKey_enterPressed(int)));

    setDialogInCenter(this);
    initialize();
}

int NetWorkSettingDialog::initialize()
{
    if(NULL==g_pConfigInfo || NULL==ui)
    {
        LOG_ERR("NULL==g_pConfigInfo || NULL==ui g_pConfigInfo:%p, ui:%p",  g_pConfigInfo, ui);
        return -1;
    }
    //get config settings
    int iRet = g_pConfigInfo->getSettings_vclient(m_settingSet_in);
    if(iRet < 0)
    {
        LOG_ERR("get config failed. return value:%d", iRet);
        memset(&m_settingSet_in, 0, sizeof(SETTINGS_VCLIENT));
        memset(&m_settingSet_out, 0, sizeof(SETTINGS_VCLIENT));
        return -5;
    }
    m_settingSet_out = m_settingSet_in;
    //put settings to the ui
    ui->lineEdit_preferenceIp->setText(m_settingSet_in.m_network.stFirstServer.serverAddress);
    ui->comboBox_protocol->setCurrentIndex(m_settingSet_in.m_network.stFirstServer.isHttps==0? 0 : 1);
    ui->lineEdit_port->setText(m_settingSet_in.m_network.stFirstServer.port);

    ui->lineEdit_alternateIp->setText(m_settingSet_in.m_network.stAlternateServer.serverAddress);
    ui->comboBox_protocol_2->setCurrentIndex(m_settingSet_in.m_network.stAlternateServer.isHttps==0? 0 : 1);
    ui->lineEdit_port_2->setText(m_settingSet_in.m_network.stAlternateServer.port);

    if(m_settingSet_in.iAutoConnectToServer==1)
        ui->checkBox_autoConnect->setChecked(true);
    else
        ui->checkBox_autoConnect->setChecked(false);

    if(CUpdate::getAutoUpdateServiceStatus()>0)
    {
        ui->radioButton_update_auto->setChecked(false);
        ui->radioButton_update_manual->setChecked(false);
        ui->radioButton_update_auto2->setChecked(true);
        m_settingSet_in.m_updateSetting = AUTO_UPDATE;
    }
    else
    {
        if(AUTO_DETECT == m_settingSet_in.m_updateSetting )
        {
            ui->radioButton_update_auto->setChecked(true);
            ui->radioButton_update_manual->setChecked(false);
            ui->radioButton_update_auto2->setChecked(false);
        }
        else
        {
            m_settingSet_in.m_updateSetting = MANUAL_UPDATE;
            ui->radioButton_update_auto->setChecked(false);
            ui->radioButton_update_manual->setChecked(true);
            ui->radioButton_update_auto2->setChecked(false);
        }
    }
#ifdef WIN32
    //    ui->pushButton_multiAccess->hide();
    if(LOUNCH_ON_START == getLaunchOnSysStart())
        ui->checkBox_lounchAppOnSysStart->setChecked(true);
    else
        ui->checkBox_lounchAppOnSysStart->setChecked(false);
#else
    ui->label_8->hide();
    ui->checkBox_lounchAppOnSysStart->hide();
#endif
    //we sholudl initialize it after setting the ui,or the value will be true
    m_bNetworkSettingChanged = false;
    m_bOtherSettingChanged = false;
    return 0;
}

NetWorkSettingDialog::~NetWorkSettingDialog()
{
    /*
    if( NULL != m_process_execCmd)
    {
        if(m_process_execCmd->state() == QProcess::Running)
            m_process_execCmd->kill();
        delete m_process_execCmd;
        m_process_execCmd = NULL;
    }
    */
    if(NULL != m_pSession)
    {
        //delete m_pSession;
        if(TASK_UUID_NULL != m_taskUuid)
            m_pSession->cancelTask(m_taskUuid);
        LOG_INFO("%s","going to delete csession");
        CThread::createThread(NULL, NULL, (FUN_THREAD)(&releaseCSession), (void*)m_pSession);
        m_pSession = NULL;
    }

    if(NULL != m_pMutex_getDomain)
    {
        delete m_pMutex_getDomain;
        m_pMutex_getDomain = NULL;
    }

    if(NULL != m_pUpdateVClientThread)
    {
        disconnect(m_pUpdateVClientThread, SIGNAL(finished()), this, SLOT(on_UpdateThreadFinished()));
        delete m_pUpdateVClientThread;
        m_pUpdateVClientThread = NULL;
    }

    if(NULL != ui)
        delete ui;
    ui = NULL;

    if(m_pMovie!=NULL)
        delete m_pMovie;
    m_pMovie = NULL;

    if(NULL != m_titleWidget)
    {
        delete m_titleWidget;
        m_titleWidget = NULL;
    }
}

void NetWorkSettingDialog::closeEvent(QCloseEvent *event)
{
    if((m_bOtherSettingChanged || m_bNetworkSettingChanged)&& false==g_bSystemShutDownSignal)
    {
        if(ACCEPTED == CMessageBox::SelectedBox(tr("Are you sure to save all the Settings?"), NULL))
        {
            int iRet = getSettings(1,1,1);
            iRet = checkValidation(true);
            if(iRet < 0)
            {
                LOG_ERR("%s", "check Validation failed.");
                event->ignore();
                return;
            }
            iRet = applySettings(1,1,1);
            if(iRet < 0)
            {
                LOG_ERR("%s", "applySetting failed");
                event->ignore();
            }
        }
        else
        {
            m_settingSet_out = m_settingSet_in;
            return;
        }
    }
}

void NetWorkSettingDialog::mouseMoveEvent(QMouseEvent *_qm)
{
    if(_qm->buttons() == Qt::LeftButton && m_isMove)
        move(_qm->globalPos() - m_pressPoint);
}

void NetWorkSettingDialog::mousePressEvent(QMouseEvent *_qm)
{
    m_isMove = true;
    m_pressPoint = _qm->pos();
}

void NetWorkSettingDialog::mouseReleaseEvent(QMouseEvent *)
{
    m_isMove = false;
}

void NetWorkSettingDialog::keyPressEvent (QKeyEvent * _qk)
{
    if(!m_isUiEnable){
        return;
    }
    if(_qk->key()==Qt::Key_Return || _qk->key() == Qt::Key_Enter )
    {
        int iPageIdx = ui->tabWidget->currentIndex();
        if(iPageIdx == 0)
            emit getKey_enterPressed(0);
        else if(iPageIdx == 1)
        {
            if(focusWidget() == ui->lineEdit_cmdParam)
                emit getKey_enterPressed(1);
        }
        else if(iPageIdx == 2)
            emit getKey_enterPressed(2);//fix bug #4296
        else
            LOG_ERR("unknown tab page:%d", iPageIdx);
    }
}

void NetWorkSettingDialog::showEvent(QShowEvent *)
{
    if ( this->isVisible())
        this->repaint();
}

void NetWorkSettingDialog::paintEvent(QPaintEvent *_paintEvent)
{
    setMaximumSize(400,350);
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, true);
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
    paint.drawRect(QRectF(0,0,width(),height()));
    //    paint.drawRoundRect(QRectF(0, 0, width()-6, height()-6), 3, 3);
    QDialog::paintEvent(_paintEvent);
}

int NetWorkSettingDialog::on_getKey_enterPressed(int type)
{
    if(0==type)
        on_pushBtn_connect_clicked();
    else if(1==type)
        on_pushBtn_executePressed();
    else if(2==type)
        on_pushBtn_apply_other_settings();
    return 0;
}

void NetWorkSettingDialog::on_comboBox_protocol_currentIndexChanged(int index)
{
    if(index == 0)
        ui->lineEdit_port->setText("80");
    else
        ui->lineEdit_port->setText("443");
}

void NetWorkSettingDialog::on_comboBox_protocol_2_currentIndexChanged(int index)
{
    if(index == 0)
        ui->lineEdit_port_2->setText("80");
    else
        ui->lineEdit_port_2->setText("443");
}

#if 0
void NetWorkSettingDialog::on_readOutput()
{
    m_str_cmdOutput += QString::fromLocal8Bit(m_process_execCmd->readAll());
    if(NULL != ui)
    {
        ui->textEdit_result->setText(m_str_cmdOutput);
        ui->textEdit_result->moveCursor(QTextCursor::End);
    }
}

int NetWorkSettingDialog::on_pushBtn_executePressed()
{
    if (m_process_execCmd != NULL)
    {
        if(m_process_execCmd->state() == QProcess::Running)
            //m_process_execCmd->kill();
            m_process_execCmd->close();
        delete m_process_execCmd;
        m_process_execCmd = NULL;
    }
    QString str;
    int idx = ui->comboBox_cmd->currentIndex();
    if(0 == idx)
        str = PING_COMMAND;
    else if(1 == idx)
        str = str + TRACERT_COMMAND + " ";
    else if(2 == idx)
        str = "nslookup ";
    else
        LOG_ERR("unknown cmd type:%d", idx);
    str.append( ui->lineEdit_cmdParam->text());
    LOG_INFO("cmd:%s", str.toUtf8().data());

    m_process_execCmd = new QProcess;
    connect(m_process_execCmd,SIGNAL(readyRead()), this, SLOT(on_readOutput()));
    m_process_execCmd->start(str);
    m_str_cmdOutput.clear();
    return ERROR_OK;
}
#else
/* 解决执行网络命令后vClient卡死的问题 */
void NetWorkSettingDialog::on_readOutput()
{
    m_str_cmdOutput += QString::fromLocal8Bit(process_cmd->readAll());
    if(NULL != ui)
    {
        ui->textEdit_result->setText(m_str_cmdOutput);
        ui->textEdit_result->moveCursor(QTextCursor::End);
    }
}

int NetWorkSettingDialog::on_pushBtn_executePressed()
{
    QString str;
    int idx = ui->comboBox_cmd->currentIndex();
    if(0 == idx)
        str = PING_COMMAND;
    else if(1 == idx)
        str = str + TRACERT_COMMAND + " ";
    else if(2 == idx)
        str = "nslookup ";
    else
        LOG_ERR("unknown cmd type:%d", idx);
    str.append( ui->lineEdit_cmdParam->text());
    LOG_INFO("cmd:%s", str.toUtf8().data());

    process_cmd = new QProcess;
    connect(process_cmd, SIGNAL(readyRead()), this, SLOT(on_readOutput()));
    process_cmd->start(str);
    m_str_cmdOutput.clear();
    return ERROR_OK;
}
#endif

int NetWorkSettingDialog::on_radioBtn_updateType_changed(bool)
{
    m_bOtherSettingChanged = true;
    qDebug()<<"==== update type changed....";
    return ERROR_OK;
}

int NetWorkSettingDialog::on_checkBox_start_type_changed(bool)
{
    m_bOtherSettingChanged = true;
    qDebug()<<"==== lounch on system start changed....";
    return ERROR_OK;
}

int NetWorkSettingDialog::on_pushBtn_apply_other_settings()
{
    if(NULL == ui)
    {
        LOG_ERR("%s", "NULL == ui");
        return -1;
    }
    ui->pushButton_apply_other_settings->setDisabled(true);
    int iRet = getSettings(0, 1, 1);
    if(iRet < 0 )
    {
        LOG_ERR("get settings failed. return value:%d", iRet);
        ui->pushButton_apply_other_settings->setEnabled(true);
        return iRet;
    }
    iRet = applySettings(0, 1, 1);
    if(iRet < 0)
    {
        LOG_ERR("apply settings failed. return value:%d", iRet);
        ui->pushButton_apply_other_settings->setEnabled(true);
        return iRet;
    }
    m_bOtherSettingChanged = false;
    ui->pushButton_apply_other_settings->setEnabled(true);
    return ERROR_OK;
}

int NetWorkSettingDialog::on_pushBtn_connect_clicked()
{    
    int iRet = checkValidation(true);
    if(iRet < 0)
    {
        return iRet;
    }

#ifndef _WIN32
    // change local redhat like Linux host's eth0 networksetting
    MultiAccesses *pMultiAccesses_con = new MultiAccesses;
    pMultiAccesses_con->readFile();
    int mA_size = pMultiAccesses_con->size();
    QString qstr_tmp;
    bool network_has_changed = false;
    rh_ifcfg *prh_ifcfg = new rh_ifcfg;
    int currentIndex = ui->comboBox_preferenceIp->currentIndex();
    qDebug() << "currentIndex: " << currentIndex;
    while (mA_size > 0) {
        if( strcmp(pMultiAccesses_con->top().AccessIp, ui->lineEdit_preferenceIp->text().toStdString().c_str()) == 0 && currentIndex==0) {
            //Let's rock & roll
            if ( strlen(pMultiAccesses_con->top().ip) > 0 ) {
                prh_ifcfg->get_value(QString("BOOTPROTO"), &qstr_tmp);
                if ( strcmp(qstr_tmp.toStdString().c_str(), "none") != 0) {
                    prh_ifcfg->set_value(QString("BOOTPROTO"), QString("none"));
                    network_has_changed = true;
                }
                prh_ifcfg->get_value(QString("IPADDR"), &qstr_tmp);
                if ( strcmp(qstr_tmp.toStdString().c_str(), pMultiAccesses_con->top().ip) != 0 ) {
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
                prh_ifcfg->get_value(QString("DNS1"), &qstr_tmp);
                if ( strcmp(qstr_tmp.toStdString().c_str(), pMultiAccesses_con->top().dns1) != 0) {
                    prh_ifcfg->set_value(QString("DNS1"), QString(pMultiAccesses_con->top().dns1));
                    network_has_changed = true;
                }
            }
            break;
        }
        if(currentIndex == 0){
            break;
        }
        pMultiAccesses_con->pop();
        mA_size--;
        currentIndex--;
        qDebug() << "currentIndex: " << currentIndex;
    }
    pMultiAccesses_con->~MultiAccesses();
    if (network_has_changed) {
        //this is usefull to all
        char cmd[50];
        memset(cmd, 0, sizeof(cmd));
        strcpy(cmd,"ifdown ");
        strcat(cmd, prh_ifcfg->getIpInfo().name);
        strcat(cmd, "; ifup ");
        strcat(cmd, prh_ifcfg->getIpInfo().name);
        if (!system(cmd))
        {
            qDebug() << "rh_ifcfg network setting succeed";
        } else {
            qDebug() << "rh_ifcfg network setting failed";
        }
        //this is usefull to redhat
        //        if (!system("ifdown eth0; ifup eth0"))
        //        {
        //            qDebug() << "rh_ifcfg network setting succeed";
        //        } else {
        //            qDebug() << "rh_ifcfg network setting failed";
        //        }
    }
    delete prh_ifcfg;
    prh_ifcfg = NULL;
#endif

    iRet = getSettings(1, 1, 1);//because when connect succeed. the dlg will be close .we shold save all the settings to file
    if(iRet < 0)
    {
        LOG_ERR("get settings failed. %d", iRet);
        return -1;
    }
    if(NULL == ui)
    {
        LOG_ERR("%s", "NULL == ui");
        return -5;
    }
    iRet = getDomain();
    if(iRet < 0)
    {
        LOG_ERR("get domain failed. %d", iRet);
        return -10;
    }
    iRet = setNetworkSettingUiEnsabled(false);
    if(iRet >= 0)
    {
        showLoadingImg(true);
    }
    ui->pushButton_cancel->show();
    return 0;
}

int NetWorkSettingDialog::on_pushButton_cancel_clicked()
{
    if(NULL == m_pMutex_getDomain)
    {
        LOG_ERR("%s", "NULL == m_pMutex_getDomain");
        showLoadingImg(false);
        setNetworkSettingUiEnsabled(true);
        ui->pushButton_cancel->hide();
        return -1;
    }
    m_pMutex_getDomain->lock();
    if(m_taskUuid != TASK_UUID_NULL)
    {
        if(NULL != m_pSession)
        {
            m_pSession->cancelTask(m_taskUuid);
            m_taskUuid = TASK_UUID_NULL;
        }
    }
    m_pMutex_getDomain->unlock();
    showLoadingImg(false);
    setNetworkSettingUiEnsabled(true);
    ui->pushButton_cancel->hide();
    //    close();
    return 0;
}

int NetWorkSettingDialog::on_updateDomainFinished(int state, const DOMAIN_DATA stDomainData)
{
    state < 0 ? m_settingSet_out.m_network = m_settingSet_in.m_network : m_settingSet_in.m_network = m_settingSet_out.m_network;
    setNetworkSettingUiEnsabled(true);
    showLoadingImg(false);
    if(state < 0)
    {
        ui->pushButton_cancel->hide();
        CMessageBox::CriticalBox(tr("Connect to Server failed"), NULL);//Update domain failure
        return -1;
    }
    else
    {
        m_settingSet_out.m_network = m_pSession->getNetwork();
        m_domain_data.vstrDomainlists = stDomainData.vstrDomainlists;
        getSettings(1, 1, 1);//get other settings user may change the setting while getting domain.
        int iRet = applySettings(1, 1, 1); //save settings
        if(iRet >= 0)
        {
            m_bNetworkSettingChanged = false;
            m_bOtherSettingChanged = false;
        }
        if(m_bShowLoginWindow)
        {
            UserLoginDlg *userLoginDlg = new UserLoginDlg(true, false);
            userLoginDlg->show();
        }
        close();
    }

    return 0;
}

int NetWorkSettingDialog::on_Changed_inNetworkSetting(QString)
{
    m_bNetworkSettingChanged = true;
    return 0;
}

int NetWorkSettingDialog::on_Changed_inNetworkSetting(bool)
{
    m_bNetworkSettingChanged = true;
    return 0;
}

int NetWorkSettingDialog::on_pushBtn_close_netCmd()
{
    close();
    return 0;
}

int NetWorkSettingDialog::on_pushBtn_updateCheckClicked()
{
    int iRet = checkValidation(true);
    if(iRet < 0)
    {
        if(iRet == ERROR_NOT_CONFIG_SERVER_IP || iRet == ERROR_PORT || iRet == ERROR_SERVER_IP_SAME
                || iRet == ERROR_PREFERENCE_IP_INVALID || iRet == ERROR_ALTERNATE_IP_INVALID)
        {
            if( NULL != ui)
                ui->tabWidget->setCurrentIndex(0);
        }
        return -1;
    }

    iRet = getSettings(1, 0, 0);
    if(iRet < 0)
    {
        return iRet;
    }

    SETTINGS_VCLIENT vclientSettings = m_settingSet_out;
    vclientSettings.m_updateSetting = MANUAL_UPDATE;
    ui->pushButton_updateCheck->setEnabled(false);
    if(NULL != m_pUpdateVClientThread)
    {
        delete m_pUpdateVClientThread;
        m_pUpdateVClientThread = NULL;
    }
    m_pUpdateVClientThread = new CUpdate(vclientSettings);
    connect(m_pUpdateVClientThread, SIGNAL(finished()), this, SLOT(on_UpdateThreadFinished()));
    m_pUpdateVClientThread->start();
    return 0;
}

int NetWorkSettingDialog::applySettings(int applyNetworkSetting, int applyAutoUpdate, int applyStartType)
{
    int iRet = ERROR_OK;
    if(applyNetworkSetting)
    {//write network settings
    }
    if(applyAutoUpdate)
    {//write auto update
        iRet = CUpdate::setAutoUpdate(m_settingSet_out.m_updateSetting == AUTO_UPDATE);
        if(iRet < 0)
        {
            CMessageBox::CriticalBox(tr("Set update type failed!"), NULL);
            return iRet;
        }
    }
    if(applyStartType)
    {//write lounch on system start
        iRet = setLaunchOnSysStart(m_settingSet_out.m_louchAppOnSysStart);
        if(iRet < 0)
            return iRet;
    }
    //save settings
    if(NULL == g_pConfigInfo)
    {
        LOG_ERR("%s","NULL == g_pConfigInfo");
        return -1;
    }
    iRet = g_pConfigInfo->setSettings_vclient(m_settingSet_out);
    if(iRet < 0)
        LOG_ERR("save settings failed. reason:%d:", iRet);
    return iRet;
}

int NetWorkSettingDialog::getSettings(int getNetworkSetting, int getAutoUpdate, int getStartType)
{
    // please make sure that all the settings are valid(call checkValidation to verify)
    if(getNetworkSetting)
    {
        //write network settings
        strcpy(m_settingSet_out.m_network.stFirstServer.serverAddress, ui->lineEdit_preferenceIp->text().toUtf8().data());
        strcpy(m_settingSet_out.m_network.stFirstServer.port, ui->lineEdit_port->text().toUtf8().data());
        strcpy(m_settingSet_out.m_network.stAlternateServer.serverAddress, ui->lineEdit_alternateIp->text().toUtf8().data());
        strcpy(m_settingSet_out.m_network.stAlternateServer.port, ui->lineEdit_port_2->text().toUtf8().data());
        if( 0 == ui->comboBox_protocol->currentIndex() )
            m_settingSet_out.m_network.stFirstServer.isHttps = 0;
        else
            m_settingSet_out.m_network.stFirstServer.isHttps = 1;
        if( 0 == ui->comboBox_protocol_2->currentIndex() )
            m_settingSet_out.m_network.stAlternateServer.isHttps = 0;
        else
            m_settingSet_out.m_network.stAlternateServer.isHttps = 1;
        m_settingSet_out.iAutoConnectToServer = ui->checkBox_autoConnect->isChecked()? 1:0;
    }
    if(getAutoUpdate)
    {//write auto update
        if(ui->radioButton_update_auto->isChecked())
            m_settingSet_out.m_updateSetting = AUTO_DETECT;
        else if (ui->radioButton_update_auto2->isChecked())
            m_settingSet_out.m_updateSetting = AUTO_UPDATE;
        else
            m_settingSet_out.m_updateSetting = MANUAL_UPDATE;
    }
    if(getStartType)
    {//write lounch on system start
        if(ui->checkBox_lounchAppOnSysStart->isChecked())
            m_settingSet_out.m_louchAppOnSysStart = LOUNCH_ON_START;
        else
            m_settingSet_out.m_louchAppOnSysStart = NOT_LOUNCH_ON_START;
    }
    return ERROR_OK;
}

LOUCH_APP_ON_SYS_START NetWorkSettingDialog::getLaunchOnSysStart()
{
    int iRet = lounchOnSystemStart(NULL, 0, 2);
    if(iRet >= 0)
    {
        return LOUNCH_ON_START;
    }
    else
        return NOT_LOUNCH_ON_START;
}

int NetWorkSettingDialog::setLaunchOnSysStart(LOUCH_APP_ON_SYS_START isStart)
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
            ui->checkBox_lounchAppOnSysStart->setCheckState(Qt::Checked);
            return -1;
        }
        else
        {
            ui->checkBox_lounchAppOnSysStart->setCheckState(Qt::Unchecked);
            return ERROR_OK;
        }
    }
    else
    {//current state:start on lounch
        if(isStart == LOUNCH_ON_START)
        {
            ui->checkBox_lounchAppOnSysStart->setCheckState(Qt::Checked);
            return ERROR_OK;
        }
        else
        {
            CMessageBox::CriticalBox(tr("Set Louch-On-System-Start Info failed"), NULL);
            ui->checkBox_lounchAppOnSysStart->setCheckState(Qt::Unchecked);
            return -1;
        }
    }

    return ERROR_OK;
}

int NetWorkSettingDialog::checkValidation(bool b_ShowMsgBox/* = false */)
{
//    if(ui->lineEdit_preferenceIp->text().isEmpty() && ui->lineEdit_alternateIp->text().isEmpty())
    if(ui->lineEdit_preferenceIp->text().isEmpty())
    {
        if(b_ShowMsgBox)
            CMessageBox::CriticalBox(tr("Information incomplete"), NULL);
        return ERROR_NOT_CONFIG_SERVER_IP;
    }
    if(0 == ui->lineEdit_preferenceIp->text().compare(ui->lineEdit_alternateIp->text()))
    {
        if(b_ShowMsgBox)
            CMessageBox::WarnBox(tr("preference and alternative Server IP is the same"), NULL);
        return ERROR_SERVER_IP_SAME;
    }

    if (!IsValidPort(ui->lineEdit_port->text()) || !IsValidPort(ui->lineEdit_port_2->text()))
    {
        if(b_ShowMsgBox)
            CMessageBox::CriticalBox(tr("Port format incorrect"), NULL);
        return ERROR_PORT;
    }

    if(!ui->lineEdit_preferenceIp->text().isEmpty() && !IsValidUrl(ui->lineEdit_preferenceIp->text()))
    {
        if(b_ShowMsgBox)
            CMessageBox::CriticalBox(tr("IP format incorrect"), NULL);
        return ERROR_PREFERENCE_IP_INVALID;
    }

    if(!ui->lineEdit_alternateIp->text().isEmpty())
    {
        if(!IsValidUrl(ui->lineEdit_alternateIp->text()))
        {
            if(b_ShowMsgBox)
                CMessageBox::CriticalBox(tr("IP format incorrect"), this);
            return ERROR_ALTERNATE_IP_INVALID;
        }
    }

    return 0;
}

int NetWorkSettingDialog::setNetworkSettingUiEnsabled(bool bEnbled)
{
    if(NULL == ui)
    {
        LOG_ERR("%s", "NULL == ui");
        return -1;
    }
    m_isUiEnable = bEnbled;
    qDebug()<<"set networksetting ui:"<<bEnbled;
    ui->lineEdit_preferenceIp->setEnabled(bEnbled);
    ui->comboBox_protocol->setEnabled(bEnbled);
    ui->lineEdit_port->setEnabled(bEnbled);

    ui->lineEdit_alternateIp->setEnabled(bEnbled);
    ui->comboBox_protocol_2->setEnabled(bEnbled);
    ui->lineEdit_port_2->setEnabled(bEnbled);

    ui->checkBox_autoConnect->setEnabled(bEnbled);

    ui->pushBtn_connect->setEnabled(bEnbled);
    ui->pushButton_multiAccess->setEnabled(bEnbled);
    ui->comboBox_preferenceIp->setEnabled(bEnbled);
    return 0;
}
int callback_networkSettingWindow(CALLBACK_PARAM_UI* pParamUi, int, void* pVoid)
{
    if( NULL==pVoid || NULL==pParamUi )
    {
        LOG_ERR("NULL==pVoid || NULL==pParamUi, pVoid:%p, pParamUi:%p", pVoid, pParamUi);
        return -1;
    }
    NetWorkSettingDialog* pNetworkSettingWindow = (NetWorkSettingDialog*)pParamUi->pUi;
    DOMAIN_DATA* pDomain = (DOMAIN_DATA*)pVoid;
    if(NULL != gpMutex_getDomain_networkSetDlg)
    {
        gpMutex_getDomain_networkSetDlg->lock();
        pNetworkSettingWindow->m_taskUuid = TASK_UUID_NULL;
        if(!gbWindowReleased_networkSetDlg)
        {
            pNetworkSettingWindow->emitUpdateDomainFinishedSignal(pParamUi->errorCode, *pDomain);
        }
        gpMutex_getDomain_networkSetDlg->unlock();
        delete gpMutex_getDomain_networkSetDlg;
        gpMutex_getDomain_networkSetDlg = NULL;
    }
    //pSettingWindow->on_updateDomainFinished(pParamUi->errorCode, *pDomain);
    delete(pDomain);
    pDomain = NULL;

    return ERROR_OK;
}

int NetWorkSettingDialog::getDomain()
{
    if( NULL != m_pSession )
    {
        //delete m_pSession;
        if(TASK_UUID_NULL != m_taskUuid)
            m_pSession->cancelTask(m_taskUuid);
        LOG_INFO("%s","going to release cession");
        CThread::createThread(NULL, NULL, (FUN_THREAD)(&releaseCSession), (void*)m_pSession);
        m_pSession = NULL;
    }
    USER_INFO userinfo;
    memset(&userinfo, 0, sizeof(USER_INFO));
    m_pSession = new CSession(m_settingSet_out.m_network, userinfo);
    if( NULL == m_pSession )
    {
        LOG_ERR("%s","NULL == m_pSession");
        return -1;
    }
    CALLBACK_PARAM_UI *cal_param = new CALLBACK_PARAM_UI;
    cal_param->pUi = this;
    PARAM_SESSION_IN param;
    param.callbackFun = callback_networkSettingWindow;
    param.callback_param = cal_param;
    param.isBlock = UNBLOCK;

    if(NULL == gpMutex_getDomain_networkSetDlg)
    {
        gpMutex_getDomain_networkSetDlg = new CMutexOp(*(getMutex()));
    }
    int iRet = m_pSession->getDomainList(param);
    if(iRet < 0)
    {
        return iRet;
    }
    m_taskUuid = param.taskUuid;
    return 0;
}

int NetWorkSettingDialog::showLoadingImg(bool bShow)
{
    if(NULL==ui || NULL==m_pMovie)
    {
        LOG_ERR("NULL==ui || NULL==m_pMovie ui:%p, m_pMovie:%p", ui, m_pMovie);
        return -1;
    }
    if(bShow)
    {
        m_pMovie->start();
        ui->label_loading->show();
    }
    else
    {
        m_pMovie->stop();
        ui->label_loading->hide();
    }
    return 0;
}

int NetWorkSettingDialog::on_UpdateThreadFinished()
{
    if(NULL != m_pUpdateVClientThread)
    {
        delete m_pUpdateVClientThread;
        m_pUpdateVClientThread = NULL;
    }
    ui->pushButton_updateCheck->setEnabled(true);
    return 0;
}

int NetWorkSettingDialog::refreshAccessIpList(void)
{
#ifndef _WIN32
    ui->comboBox_preferenceIp->clear();
    MultiAccesses *pMultiAccesses = new MultiAccesses;
    pMultiAccesses->readFile();
    int mA_size = pMultiAccesses->size();
    while (mA_size > 0) {
        if(strlen(pMultiAccesses->top().AccessIp) > 0){
            ui->comboBox_preferenceIp->addItem(pMultiAccesses->top().AccessIp);
        }
        pMultiAccesses->pop();
        mA_size--;
    }
    pMultiAccesses->~MultiAccesses();
#endif
    return 0;
}

void NetWorkSettingDialog::on_pushButton_multiAccess_clicked()
{
#ifndef _WIN32
    multiAccessesDialog *multiAccessesDialogInstance = new multiAccessesDialog(this);
    multiAccessesDialogInstance->exec();
#endif
}

