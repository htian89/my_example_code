#include "cpersonalsetting.h"
#include "ui_cpersonalsetting.h"
#include "cloadingdialog.h"
#include "ui_interact_backend.h"
#include "../common/log.h"
#include "../common/errorcode.h"
#include "../common/cconfiginfo.h"
#include "cmessagebox.h"
#include "desktoplistdialog.h"
#include <QMouseEvent>
#include "base64.h"
#include "../imageconf.h"

//Debug
#include <iostream>
using std::cerr;
using std::endl;

extern CConfigInfo* g_pConfigInfo; //defined in main.cpp
extern bool vclass_flag;

CPersonalSetting::CPersonalSetting(std::vector<VIRTUALDISK> iVDisks, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CPersonalSetting),
    m_count(0),
    m_isChangePswd(true),
    m_isMove(false),
    m_vDisks(iVDisks),
    m_loadingDlg(NULL),
    m_pSession(NULL),
    mutex_modify(NULL)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose);
    sysClose = new SysButton("icon_close.png", tr("Close"), this);
    ui->horizontalLayout_top->addWidget(sysClose);
    connect(sysClose, SIGNAL(clicked()), this, SLOT(close()));
    //ui->label_8->setPixmap(QPixmap( CORNER_TOP_LEFT_IMG ));
    ui->label_8->setPixmap(QPixmap(vclient_image.corner_left_top.data()));
    setWindowFlags( Qt::FramelessWindowHint | Qt::Dialog);
    //setWindowIcon(QIcon(WINDOWS_ICON));
    setWindowIcon(QIcon(WINDOWS_IMG.data()));
    setAttribute(Qt::WA_TranslucentBackground);
    m_backgroundPixmap.load(IMAGE_PATH_SETTING_WINDOWS_BACKGROUND);
    QString string ="QLineEdit{"
                "height:25px;"
                "width:260px;"
                "border-style:groove;"
                "border:1px groove #999999;"
                "padding: 0 8px;"
                "selection-background-color: darkgray;}"
                "QTabWidget{"
                "border:0px;}"
                "QTabBar::tab{"
                "font: bold 15px;"
                "color: rgb(0,0,0);"
                "border:0px;"
                "border-top-left-radius: 4px;"
                "border-top-right-radius: 4px;"
                "min-width: 80px;"
                "margin-left:10px;"
                "min-height: 18px;"
                "padding: 5px;}"
                "QTabBar::tab:selected{"
                "font: bold 15px;"
                "color rgb(0,0,0);"
                "background-color: #e9e9e9;}"
                "QTabBar::tab:!selected{"
                "font: bold 15px ;"
                "color rgb(0,0,0);"
                "background-color:#cccccc;}";
    //ui->tabWidget->getTabBar()->setStyleSheet(string);
    ui->tabWidget->setStyleSheet(string);

    QDialogButtonBox *z_btnBox1 = new QDialogButtonBox();
    QPushButton *pushBtn_ok = new QPushButton(tr("OK"));
    QPushButton *pushBtn_cancel = new QPushButton(tr("Cancel"));
    z_btnBox1->addButton(pushBtn_ok,QDialogButtonBox::AcceptRole);
    z_btnBox1->addButton(pushBtn_cancel,QDialogButtonBox::RejectRole);
    ui->horizontalLayout_4->addWidget(z_btnBox1);

    QDialogButtonBox *z_btnBox2 = new QDialogButtonBox();
    QPushButton *pushBtn_ok1 = new QPushButton(tr("OK"));
    QPushButton *pushBtn_cancel1 = new QPushButton(tr("Cancel"));
    z_btnBox2->addButton(pushBtn_ok1,QDialogButtonBox::AcceptRole);
    z_btnBox2->addButton(pushBtn_cancel1,QDialogButtonBox::RejectRole);
    ui->horizontalLayout_5->addWidget(z_btnBox2);
    pushBtn_ok->setFixedSize(70, 25);
    pushBtn_cancel->setFixedSize(70, 25);
    pushBtn_ok1->setFixedSize(70, 25);
    pushBtn_cancel1->setFixedSize(70, 25);
    pushBtn_ok->setStyleSheet(STYLE_SHEET_PUSHBTN);
    pushBtn_cancel->setStyleSheet(STYLE_SHEET_PUSHBTN);
    pushBtn_ok1->setStyleSheet(STYLE_SHEET_PUSHBTN);
    pushBtn_cancel1->setStyleSheet(STYLE_SHEET_PUSHBTN);

    //Diskmap setting ui
    m_lineEditFilePath = new QLineEdit;
    ui->pushBtn_apply->setStyleSheet(STYLE_SHEET_PUSHBTN);
    ui->comboBox_filepath->setStyleSheet(STYLE_SHEET_COMBO_BOX);
    ui->comboBox_filepath->setLineEdit(m_lineEditFilePath);
    ui->comboBox_filepath->setEditable(true);
    ui->pushBtn_apply->setFixedSize(70,25);
//    QPalette pal = palette();
//    pal.setBrush(QPalette::Window, QBrush(QPixmap(IMAGE_PATH_SETTING_WINDOWS_BACKGROUND)));
//    setPalette(pal);


    connect(this, SIGNAL(on_signal_modify_finished(int,int)), this, SLOT(on_modify_info_finished(int,int)));
    connect(this, SIGNAL(on_signal_modifyNt_finished(int,int)), this, SLOT(on_modify_Ntinfo_finished(int,int)));
    connect(pushBtn_ok,SIGNAL(clicked()),this,SLOT(on_pushBtn_ok_clicked()));
    connect(pushBtn_cancel,SIGNAL(clicked()),this,SLOT(on_pushBtn_cancel_clicked()));

    connect(pushBtn_ok1,SIGNAL(clicked()),this,SLOT(on_pushBtn_ok1_clicked()));
    connect(pushBtn_cancel1,SIGNAL(clicked()),this,SLOT(on_pushBtn_cancel_clicked()));
#ifdef VERSION_VSAP
    ui->label_5->setText(tr("VSAP Bar:"));
#endif
//get vdisk num and ntuserinfo
    connect(this, SIGNAL(on_signal_getUserInfo_finished(int,int,void*)), this, SLOT(on_getUserInfo_finished(int,int,void*)));
    m_pSession = CSession::GetInstance();
    memset(&m_stUserInfo, 0, sizeof(USER_INFO));
    memset(&m_accountInfo, 0, sizeof(NT_ACCOUNT_INFO));
    if(m_pSession!=NULL)
    {
        m_accountInfo = m_pSession->getNT_ACCOUNT_INFO(); //first set the value currently(it may not correct, but if we get the value from vaccess failed. at least the item in ui is not NULL)
        m_stUserInfo = m_pSession->getUSER_INFO();
        LOG_INFO("m_accountinfo:%s", m_accountInfo.ntUsername);

        CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
        if(NULL == pCall_param)
            LOG_ERR("%s", "new failed! NULL == pCall_param");
        else
        {
            pCall_param->pUi = this;
            pCall_param->uiType = PERSONALSETTING;
            memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
            PARAM_SESSION_IN param;
            param.callbackFun = uiCallBackFunc;
            param.callback_param = pCall_param;
            param.isBlock = UNBLOCK;
                setLoadingDlg(true);
            m_pSession->getUserInfo(param);
        }
    }

    //set fap file map
    memset((void*)(&m_vClientSettings), 0, sizeof(SETTINGS_VCLIENT));
    if(NULL != g_pConfigInfo)
    {
        int iRet = g_pConfigInfo->getSettings_vclient(m_vClientSettings);
        if(iRet < 0)
            LOG_ERR("get config info from file failed. return value:%d", iRet);
    }
    else
    {
        LOG_ERR("%s","NULL == g_pConfigInfo");
    }
    connect(ui->treeView_file, SIGNAL(clicked(QModelIndex)), this, SLOT(on_treeview_clicked(QModelIndex)));
    connect(ui->pushBtn_apply, SIGNAL(clicked()), this, SLOT(on_apply_clicked()));
    connect(ui->checkBox_mapdisk, SIGNAL(stateChanged(int)), this, SLOT(on_checkbox_stateChange(int)));
    if(m_vClientSettings.m_mapset == MAP_STATE){
        ui->checkBox_mapdisk->setChecked(true);
        ui->comboBox_filepath->setEnabled(true);
        ui->treeView_file->setEnabled(true);
    }else{
        ui->checkBox_mapdisk->setChecked(false);
        ui->comboBox_filepath->setEnabled(false);
        ui->treeView_file->setEnabled(false);
    }
    ui->comboBox_filepath->setMaxCount(5);
    if(strlen(m_vClientSettings.m_mapFilePathList.stFirstPath.filePath) > 0){
        cout << m_vClientSettings.m_mapFilePathList.stFirstPath.filePath << endl;
        if(QFile(QString::fromUtf8(m_vClientSettings.m_mapFilePathList.stFirstPath.filePath)).exists()){
            ui->comboBox_filepath->addItem(QString::fromUtf8(m_vClientSettings.m_mapFilePathList.stFirstPath.filePath));
        }
    }
    if(strlen(m_vClientSettings.m_mapFilePathList.stAlterNatePath.filePath) > 0){
        cout << m_vClientSettings.m_mapFilePathList.stAlterNatePath.filePath << endl;
        if(QFile(QString::fromUtf8(m_vClientSettings.m_mapFilePathList.stAlterNatePath.filePath)).exists()){
            ui->comboBox_filepath->addItem(QString::fromUtf8(m_vClientSettings.m_mapFilePathList.stAlterNatePath.filePath));
        }
    }
    if(strlen(m_vClientSettings.m_mapFilePathList.stPresentPath.filePath) >0 ){
        cout << m_vClientSettings.m_mapFilePathList.stPresentPath.filePath << endl;
        if(QFile(QString::fromUtf8(m_vClientSettings.m_mapFilePathList.stPresentPath.filePath)).exists()){
            ui->comboBox_filepath->addItem(QString::fromUtf8(m_vClientSettings.m_mapFilePathList.stPresentPath.filePath));
        }
    }
    m_fileSystemModel = new QFileSystemModel;
    m_fileSystemModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::PermissionMask);
    m_fileSystemModel->setRootPath(QDir::homePath());
    cout << "homepath: " << QDir::homePath().toUtf8().data()<< endl;
    ui->treeView_file->setModel(m_fileSystemModel);
    ui->treeView_file->setColumnHidden(1,true);
    ui->treeView_file->setColumnHidden(2,true);
    ui->treeView_file->setColumnHidden(3,true);
    ui->treeView_file->setColumnHidden(4,true);
    ui->treeView_file->setRootIndex(m_fileSystemModel->index(QDir::homePath()));
    ui->treeView_file->setSortingEnabled(true);
    ui->treeView_file->sortByColumn(1,Qt::AscendingOrder);
    ui->treeView_file->setRootIsDecorated(true);
    ui->comboBox_filepath->lineEdit()->setText(QDir::homePath());

    ui->tab_2->setVisible(false);
    ui->tabWidget->removeTab(2);

    if (vclass_flag)
    {
        //ui->tab_3->setVisible(false);
        ui->tabWidget->removeTab(1);
    }


    // In linux, we always has float bar, so we do this.
    ui->label_4->setVisible(false);
    ui->label_5->setVisible(false);
    ui->line->setVisible(false);
    ui->groupBox_vdiskSetting->resize(ui->groupBox_vdiskSetting->size().width(),300);
    ui->radioButton_fapHideBar->setVisible(false);
    ui->radioButton_fapShowBar->setVisible(false);
    ui->radioButton_rdpHideBar->setVisible(false);
    ui->radioButton_rdpShowBar->setVisible(false);

   // ui->tabWidget->setCurrentIndex(0); // this line causes some wired problems.
    m_current_tabIdx = ui->tabWidget->currentIndex();
//    connect(this, SIGNAL(on_checkBox_loadVDisks()), this, SLOT(on_checkBox_loadVDisks_finished()));
}

CPersonalSetting::~CPersonalSetting()
{
    if(NULL != m_loadingDlg)
    {
        m_loadingDlg->hide();
        delete m_loadingDlg;
        m_loadingDlg = NULL;
    }
    delete ui;
}
void CPersonalSetting::on_checkbox_stateChange(int state)
{
    if(state == Qt::Checked){
        ui->comboBox_filepath->setEnabled(true);
        ui->treeView_file->setEnabled(true);
    }else{
        ui->comboBox_filepath->setEnabled(false);
        ui->treeView_file->setEnabled(false);
    }
}

void CPersonalSetting::on_treeview_clicked(QModelIndex modelIndex)
{
    ui->comboBox_filepath->lineEdit()->setText(m_fileSystemModel->filePath(modelIndex));
}
void CPersonalSetting::on_apply_clicked()
{
    memset((void*)(&m_vClientSettings), 0, sizeof(SETTINGS_VCLIENT));
    if(NULL != g_pConfigInfo)
    {
        int iRet = g_pConfigInfo->getSettings_vclient(m_vClientSettings);
        if(iRet < 0)
            LOG_ERR("get config info from file failed. return value:%d", iRet);
    }
    if(!ui->checkBox_mapdisk->isChecked()){
        if(CMessageBox::SelectedBox(tr("Do you want to cancel disk mapping!"),this) == ACCEPTED ){
            m_vClientSettings.m_mapset = NOMAP_STATE;
            if(NULL != g_pConfigInfo)
            {
                int iRet = g_pConfigInfo->setSettings_vclient(m_vClientSettings);
                if(iRet < 0)
                    LOG_ERR("get config info from file failed. return value:%d", iRet);
            }
            else
            {
                LOG_ERR("%s","NULL == g_pConfigInfo");
            }
            return;
        }
        return;
    }else{
        if(ui->comboBox_filepath->lineEdit()->text().isEmpty()){
            CMessageBox::WarnBox(tr("Please select the file you want to map!"));
            return;
        }else if(!QFile(ui->comboBox_filepath->lineEdit()->text()).exists()){
            CMessageBox::WarnBox(tr("The path isn't exits!"));
            return;
        }
    }
    int loop;
    cout << "combox count: " << ui->comboBox_filepath->count() << endl;
    for(loop = 0; loop < ui->comboBox_filepath->count(); loop++){
        if(ui->comboBox_filepath->itemText(loop).compare(ui->comboBox_filepath->lineEdit()->text()) == 0){
            ui->comboBox_filepath->setItemText(loop, ui->comboBox_filepath->itemText(0));
            ui->comboBox_filepath->setItemText(0, ui->comboBox_filepath->lineEdit()->text());
            break;
        }
    }
    cout << "loop:" << loop << endl;
    if( loop == ui->comboBox_filepath->count() && ui->comboBox_filepath->count() < 5){
        ui->comboBox_filepath->addItem(ui->comboBox_filepath->lineEdit()->text());
    }else if(ui->comboBox_filepath->count() == 5){
        ui->comboBox_filepath->setItemText(ui->comboBox_filepath->count()-1, ui->comboBox_filepath->itemText(0));
        ui->comboBox_filepath->setItemText(0, ui->comboBox_filepath->lineEdit()->text());
    }

    m_vClientSettings.m_mapset = MAP_STATE;

    memset(m_vClientSettings.m_mapFilePathList.stPresentPath.filePath, 0, sizeof(m_vClientSettings.m_mapFilePathList.stPresentPath.filePath));
    strcpy(m_vClientSettings.m_mapFilePathList.stPresentPath.filePath, m_vClientSettings.m_mapFilePathList.stAlterNatePath.filePath);
    memset(m_vClientSettings.m_mapFilePathList.stAlterNatePath.filePath,0, sizeof(m_vClientSettings.m_mapFilePathList.stAlterNatePath.filePath));
    strcpy(m_vClientSettings.m_mapFilePathList.stAlterNatePath.filePath, m_vClientSettings.m_mapFilePathList.stFirstPath.filePath);
    memset(m_vClientSettings.m_mapFilePathList.stFirstPath.filePath, 0, sizeof(m_vClientSettings.m_mapFilePathList.stFirstPath.filePath));
    strcpy(m_vClientSettings.m_mapFilePathList.stFirstPath.filePath, ui->comboBox_filepath->lineEdit()->text().toUtf8().data());
    if(NULL != g_pConfigInfo)
    {
        int iRet = g_pConfigInfo->setSettings_vclient(m_vClientSettings);
        if(iRet < 0)
            LOG_ERR("get config info from file failed. return value:%d", iRet);
        else{
            CMessageBox::TipBox(tr("Set diskmap success!"));
        }
    }
    else
    {
        LOG_ERR("%s","NULL == g_pConfigInfo");
    }
}

bool CPersonalSetting::checkInputAvailable()
{
    if(ui->tabWidget->currentIndex()==0)
    {
        if(!ui->lineEdit_OldPwd->text().isEmpty() || !ui->lineEdit_NewPwd->text().isEmpty() || !ui->lineEdit_ConfirmPwd->text().isEmpty())
        {
           if(NULL!=m_pSession && LOGIN_TYPE_USERNAME_PASSWD==m_pSession->getLoginType() && ui->lineEdit_OldPwd->text().isEmpty())
            {
                CMessageBox::CriticalBox(tr("Old password can not be empty"),this);
                return false;
           }
            if(ui->lineEdit_NewPwd->text().isEmpty() || ui->lineEdit_NewPwd->text().count()<3)
            {
                CMessageBox::CriticalBox(tr("New password can not be empty or too short"),this);
                return false;
            }
            if(ui->lineEdit_ConfirmPwd->text() != ui->lineEdit_NewPwd->text())
            {
                CMessageBox::CriticalBox(tr("New passwords are not in accordance"),this);
                return false;
            }
            if(NULL!=m_pSession && LOGIN_TYPE_USERNAME_PASSWD==m_pSession->getLoginType() && ui->lineEdit_OldPwd->text() != QString(m_pSession->getUSER_INFO().password))
            {
                qDebug() << QString(m_pSession->getUSER_INFO().password);
                CMessageBox::CriticalBox(tr("Old password is wrong"),this);
                return false;
            }
            m_isChangePswd = true;
        }
//    }
//    else if(ui->tabWidget->currentIndex()==1)
//    {
        else
            m_isChangePswd = false;
//        if(ui->lineEdit_accountName->text().isEmpty() || ui->lineEdit_accountPwd->text().isEmpty())
//        {
            if(ui->lineEdit_accountName->text().isEmpty())
            {
                if(CMessageBox::SelectedBox(tr("Account name is empty, are you sure to modify?"),this) == REJECTED)
                    return false;
            }

            if(ui->lineEdit_accountPwd->text().isEmpty())
            {
                if(CMessageBox::SelectedBox(tr("Account password is empty, are you sure to modify?"),this) == REJECTED)
                    return false;
            }
    }

    return true;
}

void CPersonalSetting::on_pushBtn_ok_clicked()
{
    SETTINGS_LOGIN stLoginSetting;
    memset((void*)(&stLoginSetting), 0, sizeof(SETTINGS_LOGIN));
    memset((void*)(&m_vClientSettings), 0, sizeof(SETTINGS_VCLIENT));
    if(NULL != g_pConfigInfo)
    {
        int iRet = g_pConfigInfo->getSettings_login(stLoginSetting);
        if(iRet < 0)
            LOG_ERR("get config info from file failed. return value:%d", iRet);
        else
        {

        }

        iRet = g_pConfigInfo->getSettings_vclient(m_vClientSettings);
        if(iRet < 0)
            LOG_ERR("get config info from file failed. return value:%d", iRet);
        else
        {

        }
    }
    else
    {
        LOG_ERR("%s","NULL == g_pConfigInfo");
    }

    if(strlen(m_stUserInfo.domain)!=0)
    {
        close();
        return;
    }
    if(CSession::GetInstance()==NULL || !checkInputAvailable())
        return;

    setLoadingDlg(true);

    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = PERSONALSETTING;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;


    PARAM_CHANGE_USER_INFO_IN param_change_NtInfo;
    param_change_NtInfo.strNtUserName = ui->lineEdit_accountName->text().toUtf8().data();
    param_change_NtInfo.strNtPasswd = ui->lineEdit_accountPwd->text().toUtf8().data();
    if(m_isChangePswd)
    {
        param_change_NtInfo.strPasswd = ui->lineEdit_OldPwd->text().toUtf8().data();
        param_change_NtInfo.strNewPasswd = ui->lineEdit_ConfirmPwd->text().toUtf8().data();
        strcpy(m_stUserInfo.newpassword, param_change_NtInfo.strNewPasswd.c_str());
    }
    else if(NULL!=m_pSession && LOGIN_TYPE_TOKEN_AUTH==m_pSession->getLoginType())
        param_change_NtInfo.strNewPasswd = "";//when it is NULL, it will not change the passwd
    else
    {
        param_change_NtInfo.strPasswd = m_stUserInfo.password;
        param_change_NtInfo.strNewPasswd = m_stUserInfo.password;
    }
    LOG_INFO("changeNtuserInfo:%s", param_change_NtInfo.strNtUserName.c_str());

    m_pSession->changeNtUserInfo(param, param_change_NtInfo); // old vAccess API, it also change access account
//    m_pSession->changeUserInfo(param, param_change_NtInfo);  // New version Vaccess;192.168.22.73 when vaccess concurrent;

//    close();
}

void CPersonalSetting::on_pushBtn_ok1_clicked()
{
    SETTINGS_LOGIN stLoginSetting;
    memset((void*)(&stLoginSetting), 0, sizeof(SETTINGS_LOGIN));
    memset((void*)(&m_vClientSettings), 0, sizeof(SETTINGS_VCLIENT));
    if(NULL != g_pConfigInfo)
    {
        int iRet = g_pConfigInfo->getSettings_login(stLoginSetting);
        if(iRet < 0)
            LOG_ERR("get config info from file failed. return value:%d", iRet);
        else
        {
            stLoginSetting.iAttachVDisk = 0;
            if(ui->checkBox_loadVDisk->isEnabled() && Qt::Checked == ui->checkBox_loadVDisk->checkState())
            {
                stLoginSetting.iAttachVDisk = 1;
            }
            iRet = g_pConfigInfo->setSettings_login(stLoginSetting);
            if(iRet < 0)
                LOG_ERR("set config info from file failed. return value:%d, iAttachVDisk:%d", iRet, stLoginSetting.iAttachVDisk);
        }

        iRet = g_pConfigInfo->getSettings_vclient(m_vClientSettings);
        if(iRet < 0)
            LOG_ERR("get config info from file failed. return value:%d", iRet);
        else
        {
            if(ui->radioButton_rdpShowBar->isChecked())//write rdp has bar
                m_vClientSettings.m_rdpBar = HASBAR_STATE;
            else
                m_vClientSettings.m_rdpBar = NOBAR_STATE;

            if(ui->radioButton_fapShowBar->isChecked())
                m_vClientSettings.m_fapBar = HASBAR_STATE;
            else
                m_vClientSettings.m_fapBar = NOBAR_STATE;

            iRet = g_pConfigInfo->setSettings_vclient(m_vClientSettings);
            if(iRet < 0)
                LOG_ERR("set config info from file failed. return value:%d, iAttachVDisk:%d", iRet, stLoginSetting.iAttachVDisk);
        }
    }
    else
    {
        LOG_ERR("%s","NULL == g_pConfigInfo");
    }

    close();
    return;
}

void CPersonalSetting::callChangePassword()
{
    if(CSession::GetInstance()==NULL )
        return;
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = PERSONALSETTING;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    PARAM_CHANGE_USER_INFO_IN param_change_userInfo;
    //Compatible with the old interface to modify the password function;
    if(m_isChangePswd)
    {
        param_change_userInfo.strPasswd = ui->lineEdit_OldPwd->text().toUtf8().data();
        param_change_userInfo.strNewPasswd = ui->lineEdit_ConfirmPwd->text().toUtf8().data();
    }
    else if(NULL!=m_pSession && LOGIN_TYPE_TOKEN_AUTH==m_pSession->getLoginType())
        param_change_userInfo.strPasswd = "";//when it is NULL, it will not change the passwd
    else
    {
        param_change_userInfo.strPasswd = m_stUserInfo.password;
        param_change_userInfo.strNewPasswd = param_change_userInfo.strPasswd;
    }
    m_pSession->changeUserInfo(param, param_change_userInfo);
}


void CPersonalSetting::setLoadingDlg(bool enable)
{
    if(m_loadingDlg==NULL)
        m_loadingDlg = new CLoadingDialog(tr("Operate the request ..."), this);
    else
        m_loadingDlg->setText(tr("Operate the request ..."));
    QRect rect = geometry();
    m_loadingDlg->setPos(0, 5, rect.width(), rect.height()-5);
    m_loadingDlg->setMovieStop(enable^1);
    m_loadingDlg->setVisible(enable);
    repaint();
}

void CPersonalSetting::on_modify_info_finished(int errorCode, int dType)
{
    LOG_INFO("on_modify_info_finished:%d", errorCode );
    if(errorCode!=0)
    {
        setLoadingDlg(false);
        processErrorCode(errorCode, dType);
    }
    else
    {
        setLoadingDlg(false);
//        m_pSession = CSession::GetInstance();
//        if(m_pSession!=NULL)
//            m_pSession->setUserInfo(m_stUserInfo);
            close();
    }
}
void CPersonalSetting::on_modify_Ntinfo_finished(int errorCode, int dType)
{
    LOG_INFO("on_modify_Ntinfo_finished:%d", errorCode );
    if(errorCode!=0)
    {
        setLoadingDlg(false);
        processErrorCode(errorCode, dType);
    }
    else if( 0 == errorCode )
    {
        setLoadingDlg(false);
//        m_pSession = CSession::GetInstance();
//        if(m_pSession!=NULL)
//            m_pSession->setUserInfo(m_stUserInfo);
            close();
//       callChangePassword();   // Thh
    }
}

void CPersonalSetting::on_getUserInfo_finished(int errorCode, int dType, void *pRespondData)
{
    if(errorCode!=0)
    {
        setLoadingDlg(false);
        processErrorCode(errorCode, dType);
    }
    else
    {
        setLoadingDlg(false);
        GET_USER_INFO_DATA* pGetUserInfoData = (GET_USER_INFO_DATA*)pRespondData;
        if(NULL != pGetUserInfoData)
        {
            m_accountInfo = pGetUserInfoData->stNtAccountInfo;
            LOG_INFO("m_accountInfo:%s", m_accountInfo.ntUsername);
            if( pGetUserInfoData->vstVirtualDisks.size() > 0 )
            {
                m_vDisks = pGetUserInfoData->vstVirtualDisks;
            }
            delete pGetUserInfoData;
            pGetUserInfoData = NULL;
        }
    }
    qDebug() <<"domain: " <<m_stUserInfo.domain;
    //Domain user is not allowed to modify password and ntAccount information
    if(strlen(m_stUserInfo.domain)!=0)
    {
        ui->lineEdit_OldPwd->setEnabled(false);
        ui->lineEdit_NewPwd->setEnabled(false);
        ui->lineEdit_ConfirmPwd->setEnabled(false);
        ui->lineEdit_accountName->setEnabled(false);
        ui->lineEdit_accountPwd->setEnabled(false);
//        ui->pushBtn_ok->setEnabled(false);
//        ui->groupBox_pswd->setEnabled(false);
//        ui->groupBox_ntaccount->setEnabled(false);
        //ui->pushBtn_ok->setDisabled(true);//disables it. because doesnot allow modify anything
        //ATTENTION: if you enabled ui->pushBtn_ok btn; please pay attention to function on_pushBtn_ok_clicked
        //          to make sure how to deal in condition: if(strlen(m_stUserInfo.domain)!=0)
    }
    else
    {
        ui->lineEdit_accountName->setText(QString::fromUtf8(m_accountInfo.ntUsername));
        ui->lineEdit_accountPwd->setText(m_accountInfo.ntPassword);
        ui->label_noused1->hide();
        ui->label_noused2->hide();
        if(NULL!=m_pSession && LOGIN_TYPE_USERNAME_PASSWD!=m_pSession->getLoginType())
        {
            ui->lineEdit_OldPwd->setDisabled(true);
            ui->label->setDisabled(true);
        }
    }

    if(m_vDisks.size()<=0)
    {
        ui->checkBox_loadVDisk->setDisabled(true);
        ui->label_loadVDisk->setDisabled(true);
    }
    else
    {
        SETTINGS_LOGIN stLoginSetting;
        memset((void*)(&stLoginSetting), 0, sizeof(SETTINGS_LOGIN));
        int iRet = g_pConfigInfo->getSettings_login(stLoginSetting);
        if(iRet < 0)
        {
            LOG_ERR("get config info from file failed. return value:%d", iRet);
        }
        else
        {
            if(stLoginSetting.iAttachVDisk != 0)
            {
                ui->checkBox_loadVDisk->setChecked(true);
            }
            else
            {
                ui->checkBox_loadVDisk->setChecked(false);
            }
        }
    }
}

void CPersonalSetting::on_pushBtn_cancel_clicked()
{
    memset((void*)(&m_vClientSettings), 0, sizeof(SETTINGS_VCLIENT));
    if(NULL != g_pConfigInfo)
    {
        g_pConfigInfo->getSettings_vclient(m_vClientSettings);
    }
    close();
}

void CPersonalSetting::mouseMoveEvent(QMouseEvent *_qm)
{
    if(_qm->buttons() == Qt::LeftButton && m_isMove)
        move(_qm->globalPos() - m_pressPoint);
}

void CPersonalSetting::mousePressEvent(QMouseEvent *_qm)
{
    m_isMove = true;
    m_pressPoint = _qm->pos();
}

void CPersonalSetting::mouseReleaseEvent(QMouseEvent *)
{
    m_isMove = false;
}

void CPersonalSetting::processErrorCode(int errorCode, int opType)
{
    LOG_ERR("errcode:%d\t\t opType:%d", errorCode, opType);
    switch(errorCode)
    {
    case ERROR_FAIL:
    {
        CMessageBox::CriticalBox(tr("Modify information failed"),this);
        break;
    }
    default:
        showUiErrorCodeTip(errorCode);
        break;
    }
}

void CPersonalSetting::processCallBackData(int errorCode, int dType, void *pRespondData)
{
//    if( NULL == mutex_modify )
//        mutex_modify = new CMutexOp();
//    mutex_modify->lock();
    switch(dType)
    {
    case TYPE_CHANGE_USER_INFO:
    {
//        m_count++;  // when vaccess concurrent ;
        emit on_signal_modify_finished(errorCode, dType);
        if(pRespondData!=NULL)
            delete (char*)pRespondData;
        break;
    }
    case TYPE_CHANGE_NTUSER_INFO:
    {
//        m_count++;
        emit on_signal_modifyNt_finished(errorCode, dType);
        if(pRespondData!=NULL)
            delete (char*)pRespondData;
        break;
    }
    case TYPE_GETUSERINFO:
    {
        emit on_signal_getUserInfo_finished(errorCode, dType, pRespondData);
        break;
    }
    }
//    mutex_modify->unlock();
}

void CPersonalSetting::on_tabWidget_currentChanged(int index)
{

    if(strlen(m_stUserInfo.domain)!=0)
        return;
    if(m_current_tabIdx==0)//(index==1)
    {
        m_current_tabIdx = index;
        if(ui->lineEdit_OldPwd->text().isEmpty() && ui->lineEdit_NewPwd->text().isEmpty() && ui->lineEdit_ConfirmPwd->text().isEmpty())
        {
            if(ui->lineEdit_accountName->text().isEmpty())
            {
                if(CMessageBox::SelectedBox(tr("Account name is empty, are you sure to modify?"),this) == REJECTED)
                     ui->tabWidget->setCurrentIndex(0);
            }

            else if(ui->lineEdit_accountPwd->text().isEmpty())
            {
                if(CMessageBox::SelectedBox(tr("Account password is empty, are you sure to modify?"),this) == REJECTED)
                     ui->tabWidget->setCurrentIndex(0);
            }
            m_isChangePswd = false;
            return;
        }
        else if(NULL!=m_pSession && LOGIN_TYPE_USERNAME_PASSWD==m_pSession->getLoginType() && ui->lineEdit_OldPwd->text().isEmpty())
        {
            if(CMessageBox::SelectedBox(tr("Old password can not be empty, Are you sure to give up this change?"),this)==REJECTED)
                ui->tabWidget->setCurrentIndex(0);
            m_isChangePswd = false;
            return;
        }
        else if(ui->lineEdit_NewPwd->text().isEmpty() || ui->lineEdit_NewPwd->text().count()<3)
        {
            if(CMessageBox::SelectedBox(tr("New password can not be empty or too short, Are you sure to give up this change?"),this)==REJECTED)
                ui->tabWidget->setCurrentIndex(0);
            m_isChangePswd = false;
            return;
        }
        else if(ui->lineEdit_ConfirmPwd->text()!=ui->lineEdit_NewPwd->text())
        {
            if(CMessageBox::SelectedBox(tr("New passwords are not in accordance, Are you sure to give up this change?"))==REJECTED,this)
                ui->tabWidget->setCurrentIndex(0);
            m_isChangePswd = false;
            return;
        }
        else if(NULL!=m_pSession && LOGIN_TYPE_USERNAME_PASSWD==m_pSession->getLoginType() && ui->lineEdit_OldPwd->text()!=QString(m_pSession->getUSER_INFO().password))
        {
            CMessageBox::CriticalBox(tr("Old password is wrong"),this);
            ui->tabWidget->setCurrentIndex(0);
            m_isChangePswd = false;
            return;
        }
        else
            m_isChangePswd = true;
//    }

//    else if(m_current_tabIdx==1)//(index==0)
//    {
//        m_current_tabIdx = index;
        if(ui->lineEdit_accountName->text().isEmpty())
        {
            if(CMessageBox::SelectedBox(tr("Account name is empty, are you sure to modify?"),this) == REJECTED)
                ui->tabWidget->setCurrentIndex(0);
        }

        else if(ui->lineEdit_accountPwd->text().isEmpty())
        {
            if(CMessageBox::SelectedBox(tr("Account password is empty, are you sure to modify?"),this) == REJECTED)
                ui->tabWidget->setCurrentIndex(0);
        }
        m_isChangeAccount = true;
    }
    else
    {//index == 2
        m_current_tabIdx = index;
    }
}
void CPersonalSetting::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setBrush(Qt::NoBrush);
    QPen pen;
    QColor color(0, 0, 0, 50);
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
