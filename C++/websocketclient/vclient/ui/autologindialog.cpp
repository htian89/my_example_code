#include "autologindialog.h"
#include "ui_autologindialog.h"
#include "config.h"
#include <QPainter>
#include <QtGui>
#include <QMovie>
#include <QDialog>
#include "common.h"
#include "networksettingdialog.h"
#include "userlogindlg.h"
#include "desktoplistdialog.h"
#include "cmessagebox.h"
#include "ui_interact_backend.h"
#include "filepath.h"
#include <QString>
#include <../ipc/ipcclient.h>
#include "../common/errorcode.h"
#include <net/if.h>
#include <sys/ioctl.h>
#include <a.out.h>
#include <netdb.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <iostream>
#include "common/tcp_message.h"
using namespace std;
#define BUFSIZE 8192
extern CConfigInfo* g_pConfigInfo; //defined in main.cpp
extern IpcClient *g_ipcClient;  //defined in main.cpp
extern bool g_bSystemShutDownSignal;
extern bool g_bMapFileSystemOccupy;
extern AutoLoginDialog *g_autoLoginDlg;
extern bool g_autoLogin;

AutoLoginDialog  *AutoLoginDialog::s_autoLoginDlg = NULL;
QMutex AutoLoginDialog::s_mutex;

struct route_info
{
    struct in_addr dstAddr;
    struct in_addr srcAddr;
    struct in_addr gateWay;
    char ifName[IF_NAMESIZE];
};
MyThread::MyThread(void *arg, void *(*threadFunc)(void *)) :
    QThread(),
    m_arg(arg),
    m_threadFunc(threadFunc)
{
}

void MyThread::run()
{
    m_threadFunc(m_arg);
    exec();
}
AutoLoginDialog::AutoLoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AutoLoginDialog),
    m_failGetDomainCount(0),
    m_listUserResCount(0),
    m_bGetDomain(false),
    m_bLoginSession(false),
    m_canConnect(false),
    m_closeAutoLoginDialog(false),
    m_hasConnectRdp(true),
    m_hasConnectFap(false),
    m_launchApp(NULL),
    m_launchDesktop(NULL),
    m_startDesktop(NULL),
    m_pSession(NULL),
    m_mythread(NULL)
{
    memset(m_gateWay, 0, 512);
    memset(m_userName, 0, 512);
    memset(m_uuid, 0, 512);
    m_domainList.clear();
    m_stAppList.clear();
    m_stAppBakList.clear();
    m_vstVirtualDisks.clear();
    g_bMapFileSystemOccupy = false;
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint );
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute( Qt::WA_TranslucentBackground);
    setWindowTitle("vclient");
    setDialogInCenter(this);
    //setWindowIcon(QIcon(WINDOWS_ICON));
    setWindowIcon(QIcon(WINDOWS_IMG.data()));

    sysMax = new SysButton("icon_exchange.png",tr("Change"),this);
    connect(sysMax, SIGNAL(clicked()), this, SLOT(showFullScreen()));
    sysClose = new SysButton("icon_close.png", tr("Close"), this);
    connect(sysClose, SIGNAL(clicked()), this, SLOT(close()));
    ui->horizontalLayout_top->addWidget(sysMax);
    ui->horizontalLayout_top->addWidget(sysClose);
    //    ui->bottom_right->setText(tr("Ctrl + Esc logout"));
    sysClose->setVisible(false);
    sysMax->setVisible(false);
    setWindowState(windowState() | Qt::WindowFullScreen);

    //m_bakPixmap.load(IMAGE_FIRSTLOADING_BACKGROUND);
    m_bakPixmap.load(vclient_image.background.data());

    //m_backgroundPixmap = QPixmap(IMAGE_FIRSTLOADING_BACKGROUND).scaled(size());
    m_backgroundPixmap = QPixmap(vclient_image.background.data()).scaled(size());

    ui->label->setPixmap(QPixmap(":/image/resource/image/icon_info.png"));
    ui->label->setToolTip(tr("Logout(Ctrl + q)"));
    ui->label->hide();
    ui->pushButton->setToolTip(tr("Logout(Ctrl + q)"));
    ui->pushButton->setAttribute(Qt::WA_TranslucentBackground,true);
    ui->pushButton->hide();
    //m_movie = new QMovie(IMAGE_LOADING_MOVIE);
    m_movie = new QMovie(vclient_image.firstloading.data());
    if(m_movie ==NULL)
        return ;
    m_movie->setScaledSize(QSize(width()*1/4, 50*height()/m_bakPixmap.height()));
    ui->label_gif->setMovie(m_movie);
    m_movie->stop();
    ui->label_gif->setVisible(false);
    ui->label_text->setStyleSheet("font: bold 18px;color: #ffffff;");

    connect(this, SIGNAL(autologinFinished(int,int)), this,SLOT(on_autoLogin_finished(int,int)));
    connect(this, SIGNAL(on_signal_launch_desktop_finished(int, int, QString,LAUNCH_DESKTOP_DATA*)),
            this, SLOT(on_launch_desktop_finished(int, int, QString,LAUNCH_DESKTOP_DATA*)));
    connect(this, SIGNAL(on_signal_selfService_finished(int, int, QString, int, int)),
            this, SLOT(on_selfService_finished(int, int, QString, int,int)));
    connect(this, SIGNAL(on_signal_logout_finished(int, int)), this, SLOT(on_logout_finish(int, int)));
    //connect(this, SIGNAL(on_signal_setSeatNumber()), this, SLOT(on_setSeatNumber()));
   // connect(this, SIGNAL(on_signal_showSeatNumber()), this, SLOT(on_showSeatNumber()));
    //connect(this, SIGNAL(on_signal_setSeatNumber_finished()), this, SLOT(on_finish_setSeatNumber()));
    //connect(this, SIGNAL(on_signal_showNotes()), this, SLOT(on_showNotes()));
    s_autoLoginDlg = this;
    g_autoLogin = true;
    setText(tr("Launch network..."));
    setmovieStop(false);
    //get gateway to check the network, but it will block the ui, so create thread;
    m_mythread = new MyThread(this, init);
    connect(this, SIGNAL(failedtogetip()), this, SLOT(on_failed_getGateway()));
    m_mythread->start();
}
void AutoLoginDialog::on_failed_getGateway()
{
    CMessageBox::WarnBox(tr("Network anomalies, please check the network connection"),this);
    close();
}
void *AutoLoginDialog::init(void *arg)
{
    AutoLoginDialog *dlg = static_cast<AutoLoginDialog *>(arg);
    if(dlg == NULL){
        return NULL;
    }
    dlg->initGateWay();
    return NULL;
}

int AutoLoginDialog::initGateWay()
{
    if(NULL == g_pConfigInfo)
    {
        LOG_ERR("%s", "parameter error:NULL == g_pConfigInfo");
        return -1;
    }
    memset(&m_vClientSettings, 0, sizeof(m_vClientSettings));
    int iRet = g_pConfigInfo->getSettings_vclient(m_vClientSettings);
    if(iRet < 0)
    {
        LOG_ERR("%s", "get config info from file failed");
        memset(&m_vClientSettings, 0, sizeof(SETTINGS_VCLIENT));
    }
    while(1){
        if(get_gatewayip(m_gateWay, 512) > 0){
            break;
        }else{
            Sleep(2000);
            continue;
        }
    }
//    memset(m_gateWay, 0, 512);
//  strcpy(m_gateWay, "192.168.42.200");
    strcpy(m_vClientSettings.m_network.stFirstServer.serverAddress, m_gateWay);
    strcpy(m_vClientSettings.m_network.stFirstServer.port, "80");
    m_vClientSettings.m_network.stFirstServer.isHttps = 0;

    strcpy(m_vClientSettings.m_network.stPresentServer.serverAddress, m_gateWay);
    strcpy(m_vClientSettings.m_network.stPresentServer.port, "80");
    m_vClientSettings.m_network.stPresentServer.isHttps = 0;

    g_pConfigInfo->setSettings_vclient(m_vClientSettings);

    memset(m_uuid, 0, sizeof(m_uuid));
    getUuid(m_uuid);
    initDomain();
    return 0;
}
int AutoLoginDialog::readNlSock(int sockFd, char *bufPtr, size_t buf_size, int seqNum, int pId)
{
    struct nlmsghdr *nlHdr;
    int readLen = 0, msgLen = 0;

    do
    {
        /* Recieve response from the kernel */
        if((readLen = recv(sockFd, bufPtr, buf_size - msgLen, 0)) < 0)
        {
            perror("SOCK READ: ");
            return -1;
        }

        nlHdr = (struct nlmsghdr *)bufPtr;

        /* Check if the header is valid */
        if((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))
        {
            perror("Error in recieved packet");
            return -1;
        }

        /* Check if the its the last message */
        if(nlHdr->nlmsg_type == NLMSG_DONE)
        {
            break;
        }
        else
        {
            /* Else move the pointer to buffer appropriately */
            bufPtr += readLen;
            msgLen += readLen;
        }

        /* Check if its a multi part message */
        if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)
        {
            /* return if its not */
            break;
        }
    }
    while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));

    return msgLen;
}

/* parse the route info returned */
int AutoLoginDialog::parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo)
{
    struct rtmsg *rtMsg;
    struct rtattr *rtAttr;
    int rtLen;

    rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

    /* If the route is not for AF_INET or does not belong to main routing table then return. */
    if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
        return -1;

    /* get the rtattr field */
    rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
    rtLen = RTM_PAYLOAD(nlHdr);

    for(; RTA_OK(rtAttr,rtLen); rtAttr = RTA_NEXT(rtAttr,rtLen))
    {
        switch(rtAttr->rta_type)
        {
        case RTA_OIF:
            if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
            break;

        case RTA_GATEWAY:
            memcpy(&rtInfo->gateWay, RTA_DATA(rtAttr), sizeof(rtInfo->gateWay));
            break;

        case RTA_PREFSRC:
            memcpy(&rtInfo->srcAddr, RTA_DATA(rtAttr), sizeof(rtInfo->srcAddr));
            break;

        case RTA_DST:
            memcpy(&rtInfo->dstAddr, RTA_DATA(rtAttr), sizeof(rtInfo->dstAddr));
            break;
        }
    }

    return 0;
}

// meat
int AutoLoginDialog::get_gatewayip(char *gatewayip, socklen_t size)
{
    int found_gatewayip = 0;

    struct nlmsghdr *nlMsg;
    struct rtmsg *rtMsg;
    struct route_info route_info;
    char msgBuf[BUFSIZE]; // pretty large buffer

    int sock, len, msgSeq = 0;

    /* Create Socket */
    if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
    {
        perror("Socket Creation: ");
        return(-1);
    }

    /* Initialize the buffer */
    memset(msgBuf, 0, sizeof(msgBuf));

    /* point the header and the msg structure pointers into the buffer */
    nlMsg = (struct nlmsghdr *)msgBuf;
    rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);

    /* Fill in the nlmsg header*/
    nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.
    nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .

    nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
    nlMsg->nlmsg_seq = msgSeq++; // Sequence of the message packet.
    nlMsg->nlmsg_pid = getpid(); // PID of process sending the request.

    /* Send the request */
    if(send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0)
    {
        fprintf(stderr, "Write To Socket Failed...\n");
        return -1;
    }

    /* Read the response */
    if((len = readNlSock(sock, msgBuf, sizeof(msgBuf), msgSeq, getpid())) < 0)
    {
        fprintf(stderr, "Read From Socket Failed...\n");
        return -1;
    }

    /* Parse and print the response */
    for(; NLMSG_OK(nlMsg,len); nlMsg = NLMSG_NEXT(nlMsg,len))
    {
        memset(&route_info, 0, sizeof(route_info));
        if ( parseRoutes(nlMsg, &route_info) < 0 )
            continue;  // don't check route_info if it has not been set up

        // Check if default gateway
        if (strstr((char *)inet_ntoa(route_info.dstAddr), "0.0.0.0"))
        {
            // copy it over
            inet_ntop(AF_INET, &route_info.gateWay, gatewayip, size);
            found_gatewayip = 1;
            break;
        }
    }

    ::close(sock);

    return found_gatewayip;
}
int AutoLoginDialog::initConfig()
{
    if(NULL == g_pConfigInfo)
    {
        LOG_ERR("%s", "parameter error:NULL == g_pConfigInfo");
        return -1;
    }
    memset(&m_loginSettings, 0, sizeof(SETTINGS_LOGIN));
    int iRet = g_pConfigInfo->getSettings_login(m_loginSettings);
    if(iRet < 0)
    {
        LOG_ERR("get config info from file failed. return value:%d", iRet);
        memset(&m_loginSettings, 0, sizeof(SETTINGS_LOGIN));
        return -6;
    }
    strcpy(m_loginSettings.stUserInfo.username, m_userName);
    strcpy(m_loginSettings.stUserInfo.password, "123@qwe");
    strcpy(m_loginSettings.stUserInfo.uuid, m_uuid);

    g_pConfigInfo->setSettings_login(m_loginSettings);

    iRet = loginSession();
    return iRet;
}
// auto login the param need the uuid; it will be create at ~/.vclient/uuid
int AutoLoginDialog::getUuid(char _uuid[512]){
    QString uuidPath(userPath.data());
    uuidPath = uuidPath + "uuid";
    FILE *file = NULL;
    file = fopen(uuidPath.toUtf8().data(), "r");
    if(file == NULL)
    {
        QUuid uuid = QUuid::createUuid();
        QString string = uuid.toString();
        strcpy(_uuid, string.toLocal8Bit().data());
        file = fopen(uuidPath.toUtf8().data(), "w");
        fprintf(file, "%s", _uuid);
    }
    else
        if(NULL == fgets(_uuid, 256, file)){
			LOG_ERR("%s", "get uuid file error");
		}
    fclose(file);
    return 0;
}

//get username from access  login;
int AutoLoginDialog::callGetUserName()
{
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return -1;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = AUTOLOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    setText(tr("get user name..."));
    int iRet = m_pSession->getUserName(param, m_uuid);
    return iRet;
}
// write the username to ~/.vclient/username, the second login direct to use;
int AutoLoginDialog::getUserName()
{
    char userNamePath[512];
    memset(userNamePath, 0, 512);
    strcpy(userNamePath, userPath.data());
    strcat(userNamePath, "user_name");
    QString text;
    char userName[512];
    memset(userName, 0, 512);
//    bool ok;
    int iRet;
    QFile file(userNamePath);
    if(file.exists()){
        if(file.open(QIODevice::ReadOnly)){
            if(file.readLine(userName, 512)<= 0){
                file.close();
                iRet = -1;
            }else{
                file.close();
                cout << "userName" << userName << endl;
                if(userName[strlen(userName)] == '\n'){
                    userName[strlen(userName)] = '\0';
                }
                memset(m_userName, 0, sizeof(m_userName));
                strcpy(m_userName, userName);
                iRet = 0;
            }
        }else{
            iRet = -1;
        }

    }else{
        //*************************************
        //get username :
        //plan1: user input,
        //plan2: get from acess;
        //*************************************
        //       QInputDialog inputDlg;
        //       inputDlg.setWindowFlags(Qt::Popup/*|Qt::WindowStaysOnTopHint*/);
        //       text =  inputDlg.getText( NULL,
        //                    tr( "Config Login Name"),
        //                    tr( "Please enter the user name that you want login in, just config first." ),
        //                    QLineEdit::Normal, tr( "Login name" ), &ok );
        //       if(ok && !text.isEmpty()){
        //           if(file.open(QIODevice::WriteOnly))
        //           {
        //               file.close();
        //               if(file.open(QIODevice::WriteOnly))
        //               {
        //                   if(file.write(text.toUtf8().data()) <= 0)
        //                   {
        //                       LOG_ERR("Write fail");
        //                       file.close();
        //                       iRet = -1;
        //                   }else{
        //                      file.close();
        //                      strcpy(m_userName, text.toUtf8().data());
        //                      iRet = 0;
        //                   }
        //               }else{
        //                   LOG_ERR("Open error");
        //                   iRet = -1;
        //               }
        //           }
        //           else{
        //                LOG_ERR("Open error");
        //               iRet = -1;
        //           }
        //       }else{
        //          iRet = -1;
        //       }
        iRet = -1;
    }
    return iRet;
}

int AutoLoginDialog::initDomain()
{
    m_bLoginSession = false;
    showFullScreen();
    m_domainTaskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
     m_pSession->setSessionInfo(&(m_vClientSettings.m_network), NULL);
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return -1;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = AUTOLOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;

    setText(tr("Launch server..."));
    setmovieStop(false);
    int iRet = m_pSession->getDomainList(param);
    m_domainTaskUuid = param.taskUuid;
    return iRet;
}
int AutoLoginDialog::loginSession()
{
    taskUUID taskUuid = TASK_UUID_NULL;
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return -1;
    }
    setText("logining...");
    pCall_param->pUi = this;
    pCall_param->uiType = AUTOLOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    setText(tr("Login..."));
    m_pSession->setSessionInfo(&(m_vClientSettings.m_network), &(m_loginSettings.stUserInfo));
    int iRet = m_pSession->loginSession(param, NULL, true);
    return iRet;
}
int AutoLoginDialog::callListUserRes()
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
    pCall_param->uiType = AUTOLOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    setText(tr("list user resource..."));
    return m_pSession->listUserResource(param);
}

int AutoLoginDialog::callGetUserInfo()
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
    pCall_param->uiType = AUTOLOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    setText(tr("get user info..."));
    return m_pSession->getUserInfo(param);
}
/**
 * @brief AutoLoginDialog::catchDesktop
 * @desc judge the vmstate > 0 launchapp; < 0 wait calllistuserres; = 0 startdesktop
 * @return no use
 */
int AutoLoginDialog::catchDesktop()
{
    m_isConnected = false;
    m_isStarting = false;
    for(std::vector<APP_LIST>::size_type i=0; i<m_stAppList.size(); ++i)
    {
        APP_LIST *appInfo = new APP_LIST;
        memset(appInfo, 0, sizeof(APP_LIST));
        appInfo = &m_stAppList[i];
        switch((appInfo->desktopType))
        {
        case DESKTOPPOOL:
        {
            if(appInfo->sourceType==0)
            {
                if(appInfo->userAssignment==0 || appInfo->userAssignment ==2)
                {
                    if(appInfo->vmState>0 || appInfo->rdpServiceState>0)
                    {
                        m_isConnected = true;
                        m_launchDesktop = appInfo;
                        setText(tr("Launch desktop..."));
                        launchDesktop();
                    }
                    else if(appInfo->powerOnVmNum>=0 || appInfo->rdpOnVmNum>=0)
                    {
                        m_isConnected = true;
                        m_launchDesktop = appInfo;
                        setText(tr("Launch desktop..."));
                        launchDesktop();
                    }else if(appInfo->vmState == 0){
                        m_isStarting = true;
                        m_startDesktop = appInfo;
                        setText(tr("Start desktop..."));
                        startDesktop();
                    }else{
                        pthread_t thread;
                        VCLIENT_THREAD_CREATE(thread, NULL, waitforDesktopstateChange, this);
                        THREAD_DETACH(thread);
                    }
                }
            }
            break;
        }
        case REMOTEDESKTOP:
        {
            if(appInfo->vmState >0)
            {
                m_isConnected = true;
                m_launchDesktop = appInfo;
                setText(tr("Launch desktop..."));
                launchDesktop();
            }else if(appInfo->vmState == 0){
                m_isStarting = true;
                m_startDesktop = appInfo;
                setText(tr("Start desktop..."));
                startDesktop();
            }else{
                pthread_t thread;
                VCLIENT_THREAD_CREATE(thread, NULL, waitforDesktopstateChange, this);
                THREAD_DETACH(thread);
            }
            break;
        }
		default:
			break;
        }
        if(m_isConnected || m_isStarting)
        {
            break;
        }
    }
    return 0;
}
void *AutoLoginDialog::waitforDesktopstateChange(void *arg)
{
    if(arg == NULL){
        return NULL;
    }
    AutoLoginDialog *alg = static_cast<AutoLoginDialog *>(arg);
    Sleep(3000);
    alg->callListUserRes();
    return NULL;
}

void AutoLoginDialog::on_keepSession_faield(int errorCode, int dType)
{
//    on_logout_finish(errorCode, dType);
    LOG_INFO("keepsession error : %d, %d", errorCode, dType);
    showFullScreen();
    if(NULL != m_launchApp)
    {
        CLaunchApp *launchApp = m_launchApp;
        m_launchApp = NULL;
        delete launchApp;
    }
    if(NULL != m_pSession)
    {
        m_pSession = NULL;
        CSession::Release();
    }
    Sleep(1000);
    m_hasConnectFap = false;
    m_hasConnectRdp = true;
    initDomain();
}

int AutoLoginDialog::launchDesktop()
{
    if(CSession::GetInstance()==NULL)
        return -1;

    if(m_launchApp==NULL)
        m_launchApp = new CLaunchApp();

    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return -2;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = AUTOLOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));

    PARAM_LAUNCH_COMMON_IN stLaunchCommon;
    stLaunchCommon.isBlock = UNBLOCK;
    stLaunchCommon.pFunUi = uiCallBackFunc;
    stLaunchCommon.pCallbackUi_param = pCall_param;

    if(!m_hasConnectRdp)
    {
        m_hasConnectRdp = true;
        m_hasConnectFap = false;
        LAUCH_RDP stLaunchRdp;
        stLaunchRdp.barStatus = m_vClientSettings.m_rdpBar;
        if(UNKNOWN_BAR_STATUS == stLaunchRdp.barStatus)
            stLaunchRdp.barStatus = HASBAR_STATE;
        stLaunchRdp.stAppInfo = *m_launchDesktop;
        //        stLaunchRdp.stPort = item->getData().ui_status.stPort;
        stLaunchRdp.bMapFileSystem = true;
        if( m_vstVirtualDisks.size() > 0)
        {
            stLaunchRdp.launchDisk = m_vstVirtualDisks.at(0);
            LOG_INFO("m_vDisks:%s", stLaunchRdp.launchDisk.devicePath);
        }
        if(!g_bMapFileSystemOccupy )// && item->getData().appData->resParams.disk==1)
        {
            g_bMapFileSystemOccupy = true;
            stLaunchRdp.bMapFileSystem = true;
        }
        if((m_launchDesktop->desktopType==DESKTOPPOOL && m_launchDesktop->sourceType==0)
                || m_launchDesktop->desktopType ==REMOTEDESKTOP)
        {
            if(m_vstVirtualDisks.size()<=0)
            {
                LOG_ERR("%s","the user doesnot have any vDisks");
                stLaunchRdp.bAttachDisk = 0;
            }
            else
                stLaunchRdp.bAttachDisk = m_loginSettings.iAttachVDisk==0?false:true;//item->getData().ui_status.hasVDisk;
        }
        else
            stLaunchRdp.bAttachDisk = 0;
        m_launchApp->launchRdp(stLaunchCommon, stLaunchRdp);
        hide();
    }
    else if (!m_hasConnectFap)
    {
        m_hasConnectFap = true;
        m_hasConnectRdp = false;
        LAUCH_FAP stLaunchFap;

        stLaunchFap.mapStatus = m_vClientSettings.m_mapset;
        if(UNKNOWN_MAP_STATUS == stLaunchFap.mapStatus){
            stLaunchFap.mapStatus = NOMAP_STATE;
            stLaunchFap.bMapFileSystem = false;
        }else if( MAP_STATE == stLaunchFap.mapStatus){
            if(!g_bMapFileSystemOccupy )
            {
                g_bMapFileSystemOccupy = true;
                stLaunchFap.bMapFileSystem = true;
                strcpy(stLaunchFap.stFileInfo.stFirstPath.filePath,m_vClientSettings.m_mapFilePathList.stFirstPath.filePath);
                LOG_INFO("filepath: %s===", m_vClientSettings.m_mapFilePathList.stFirstPath.filePath);
            }
        }

        stLaunchFap.barStatus = m_vClientSettings.m_fapBar;
        if(UNKNOWN_BAR_STATUS == stLaunchFap.barStatus)
            stLaunchFap.barStatus = HASBAR_STATE;
        stLaunchFap.stAppInfo = *m_launchDesktop;
        if( m_vstVirtualDisks.size() > 0)
        {
            stLaunchFap.launchDisk = m_vstVirtualDisks.at(0);
            LOG_INFO("m_vDisks:%s", stLaunchFap.launchDisk.devicePath);
        }

        if((m_launchDesktop->desktopType==DESKTOPPOOL && m_launchDesktop->sourceType==0)
                || m_launchDesktop->desktopType ==REMOTEDESKTOP)
        {
            if(m_vstVirtualDisks.size()<=0)
            {
                LOG_ERR("%s","the user doesnot have any vDisks");
                stLaunchFap.bAttachDisk = 0;
            }
            else
                stLaunchFap.bAttachDisk = m_loginSettings.iAttachVDisk==0?false:true;//item->getData().ui_status.hasVDisk;
        }
        else
            stLaunchFap.bAttachDisk = 0;
        m_launchApp->launchFap(stLaunchCommon, stLaunchFap);
        hide();
    }
    return 0;
}
int AutoLoginDialog::startDesktop()
{
    if(NULL == m_pSession)
        m_pSession =CSession::GetInstance();
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return -1;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = AUTOLOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    if(m_startDesktop->desktopType == DESKTOPPOOL)
        m_pSession->startDesktopPool(param, m_startDesktop->uuid);
    else
        m_pSession->powerOnDesktop(param, m_startDesktop->uuid);
    return 0;
}

void AutoLoginDialog::on_autoLogin_finished(int errorCode, int opType)
{
    if(errorCode==0)
    {
        if(TYPE_GET_USERNAME == opType){
            initConfig();
        }else if(TYPE_GETDOMAIN == opType)
        {
            if(g_ipcClient != NULL)
                g_ipcClient->sendWebsocketNetworkInfo(m_vClientSettings.m_network.stPresentServer.serverAddress, m_vClientSettings.m_network.stPresentServer.port);
            m_loginSettings.stUserInfo.domain[0] = '\0';
            m_bGetDomain = true;
            int iRet = getUserName();
            if( iRet == 0){
                initConfig();
            }else{
                callGetUserName();
            }
        }
        else if(TYPE_LOGIN == opType|| TYPE_AUTH_TOKEN == opType)
        {
            m_bLoginSession = true;
            setText(tr("Getting resource lists ..."));
            callGetUserInfo();
        }
        else if(TYPE_GETUSERINFO == opType)
        {
            setText(tr("Getting user informations..."));
            callListUserRes();
        }
        else if(TYPE_LIST_USER_RES == opType)
        {
            catchDesktop();
        }
        else
        {
            LOG_ERR("Unknown optype:%d", opType);
        }
    }
    else
    {
        if(m_bLoginSession){
            logoutSession(BLOCKED);
        }
        if(NULL != m_pSession)
        {
            m_pSession = NULL;
            CSession::Release();
        }
        Sleep(1000);
        initDomain();
    }
}

void AutoLoginDialog::on_selfService_finished(int errorCode, int dType, QString uuid, int vmState, int rdpState)
{
	if(errorCode != 0 && !uuid.isEmpty()){
		LOG_INFO("dType selfservice failed, errorCode %d", errorCode);
	}
    if(m_startDesktop!=NULL)
    {
        if(vmState>0 || rdpState>0)
        {
            LOG_INFO("ITEM_ENABLE");
            m_launchDesktop = m_startDesktop;
            setText(tr("Launch desktop.."));
            launchDesktop();
        }
        else
        {
            LOG_INFO("ITEM_UNABLE");
            logoutSession(BLOCKED);
            if(NULL != m_pSession)
            {
                m_pSession = NULL;
                CSession::Release();
            }
            Sleep(2000);
            initDomain();
        }
    }
}

void AutoLoginDialog::on_launch_desktop_finished(int errorCode, int dType, QString uuid, LAUNCH_DESKTOP_DATA* launchData)
{
    if(NULL !=launchData )
    {
        setText(" ");
        setmovieStop(true);
        LOG_INFO("type:%d",launchData->iOpType);
        if(launchData->iOpType == TYPE_BEGIN_ATTACH_DISK)
        {
            delete launchData;
            launchData  = NULL;
            return;
        }
        else if(launchData->iOpType == TYPE_LAUNCH_ATTACH_DISK_FAILED)
        {
            enum desktoptype desktopType = VIRTUALAPP;
            if(NULL != m_launchDesktop)
            {
                desktopType = m_launchDesktop->desktopType;
            }
            if(DESKTOPPOOL == desktopType && 1==m_launchDesktop->userAssignment)
            {
                //                if( 0 < m_vDisks.size() )
                //                item->getData().vDisk = m_vDisks.at(0);  // refresh the virtualDisk.devicepath;
                dealAttachDisk_desktoppool(uuid, m_launchDesktop);
            }
            else
            {
                if(errorCode!=0)
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
            delete launchData;
            launchData  = NULL;
            return;
        }
        else if(launchData->iOpType == TYPE_DESKTOP_NOT_AVAILABLE)
        {
            if(NULL != m_launchDesktop)
            {
                APP_LIST* pAppData = m_launchDesktop;
                if(NULL != pAppData)
                {//update the desktop data
                    pAppData->rdpOnVmNum = launchData->stAppInfo.rdpOnVmNum;
                    pAppData->rdpServiceState = launchData->stAppInfo.rdpServiceState;
                    pAppData->vmState = launchData->stAppInfo.vmState;
                    pAppData->powerOnVmNum = launchData->stAppInfo.powerOnVmNum;
                    pAppData->displayprotocol = launchData->stAppInfo.displayprotocol;
                }
            }
            LOG_ERR("TYPE_DESKTOP_NOT_AVAILABLE: iRet:", errorCode);
            processErrorCode(ERROR_FAIL, TYPE_DESKTOP_NOT_AVAILABLE);
            delete launchData;
            launchData  = NULL;
            errorCode = 0;
            //return;
        }else if(launchData->iOpType == TYPE_LAUNCH_SHUTDOWN_RES_FINISHED){
            delete launchData;
            launchData  = NULL;
            LOG_INFO("desktop has disconnect showfullscreen and logout , release session");
            showFullScreen();
            logoutSession(BLOCKED);
            LOG_INFO("logout finished");
            if(NULL != m_pSession)
            {
                m_pSession = NULL;
                CSession::Release();
            }
            LOG_INFO("release session finished");
            Sleep(1000);
            m_hasConnectFap = false;
            m_hasConnectRdp = true;
            initDomain();
            LOG_INFO("initdomain ");
            return ;
        }else
        {
            LOG_ERR(" unknown type:%d",launchData->iOpType);
            delete launchData;
            launchData  = NULL;
        }
    }
    if(errorCode!=0)
    {
        if(NULL != m_launchApp)
        {
            std::string str_uuid;
            str_uuid = uuid.toUtf8().data();
            if(m_launchApp->appExit(str_uuid, (LAUNCH_TYPE)(m_hasConnectFap ? FAP_CONNECT : RDP_CONNECT)) <=0)
            {
                LOG_INFO("desktop has not quit, return directly. desktop uuid:%s", uuid.toUtf8().data());
                return;
            }
        }
        showFullScreen();
        logoutSession(BLOCKED);
        if(NULL != m_pSession)
        {
            m_pSession = NULL;
            CSession::Release();
        }
        Sleep(1000);
        m_hasConnectFap = false;
        m_hasConnectRdp = true;
        initDomain();
    }
}
void AutoLoginDialog::on_logout_finish(int errorCode, int dType)
{
    m_bLoginSession = false;
    LOG_ERR("Logout errorCode:%d, Type: %d\n", errorCode, dType);
}

int AutoLoginDialog::dealAttachDisk_desktoppool(const QString uuid, APP_LIST *appInfo)
{

    int iRet = 0;
    CSelectDialog* pSelectDlg = new CSelectDialog(tr("don't attach the virtual disk, connect to\nthe desktop directly."),\
                                                  tr("connect to a new desktop and try to load\nvirtual disk again."),\
                                                  tr("Attach virtual disk failed!"), tr("vClient"));
    st_reAttachVDisk_Dlg_Info stVdiskDlgInfo;
    stVdiskDlgInfo.launchType = m_hasConnectFap ? LAUNCH_TYPE_FAP:LAUNCH_TYPE_RDP;
    stVdiskDlgInfo.pSelDlg = pSelectDlg;
    m_hashSelectDlg.insert(uuid, stVdiskDlgInfo);
    pSelectDlg->exec();
    LOG_INFO("%s", "m_hashSelectDlg dlg closed");
    int iSelect = pSelectDlg->getSelection();

    bool hasFound = false;
    for(QHash<QString, st_reAttachVDisk_Dlg_Info>::iterator iter=m_hashSelectDlg.begin(); iter != m_hashSelectDlg.end();)
    {
        hasFound = true;
        m_hashSelectDlg.erase(iter++);
    }
    delete pSelectDlg;
    pSelectDlg = NULL;
    if(!hasFound)
    {
        LOG_INFO("%s","hasFound is false, going to exit directly");
        return 0;
    }
    LOG_INFO("desktoppool attach disk selection:%d", iSelect);
    if(NULL==m_launchApp || NULL==appInfo)
    {
        LOG_ERR("NULL==m_launchApp || NULL==item. item:%d, m_launchApp:%d", NULL==appInfo?0:1, NULL==m_launchApp?0:1);
        return -1;
    }
    if(0 == iSelect)
    {
        iRet = m_launchApp->reloadVDiskInNewDesktop(uuid.toUtf8().data(), RELOAD_VDISK_NO, (LAUNCH_TYPE)(m_hasConnectFap ? FAP_CONNECT :RDP_CONNECT));
    }
    else
    {
        iRet = m_launchApp->reloadVDiskInNewDesktop(uuid.toUtf8().data(), RELOAD_VDISK_YES, (LAUNCH_TYPE)(m_hasConnectFap ? FAP_CONNECT :RDP_CONNECT));
    }
    return iRet;
}
void AutoLoginDialog::processCallBackData(int errorCode, int dType, void *pRespondData)
{
    switch(dType)
    {
    case TYPE_GET_USERNAME:
    {
        USERNAME_DATA *pUserName = (USERNAME_DATA *)pRespondData;
        cout << "username : " << pUserName->userName << endl;

        if(errorCode != 0 ){
            if(pUserName != NULL)
            {
                delete pUserName;
                pUserName = NULL;
            }
            Sleep(3000);
            if(g_autoLoginDlg != NULL){
                callGetUserName();
            }
            break;
        }
        memset(m_userName, 0 , sizeof(m_userName));
        if(pUserName != NULL){
            strcpy(m_userName, pUserName->userName);
            delete pUserName;
        }
        char userNamePath[512];
        memset(userNamePath, 0, 512);
        strcpy(userNamePath, userPath.data());
        strcat(userNamePath, "user_name");
        QFile file(userNamePath);
        int iRet = 0;
        if(file.open(QIODevice::WriteOnly))
        {
            file.close();
            if(file.open(QIODevice::WriteOnly))
            {
                if(file.write(m_userName) <= 0)
                {
                    LOG_ERR("Write fail");
                    file.close();
                    iRet = -1;
                }else{
                    file.close();
                    iRet = 0;
                }
            }else{
                LOG_ERR("Open error");
                iRet = -1;
            }
        }
        else{
            LOG_ERR("Open error");
            iRet = -1;
        }
        if(iRet == 0){
            emit autologinFinished(errorCode, dType);
        }else{
            //failed to write username,default;
            LOG_ERR("write user_name failed");
            emit autologinFinished(errorCode, dType);
        }
        break;
    }
    case TYPE_GETDOMAIN:
    {
        m_domainTaskUuid = TASK_UUID_NULL;
        DOMAIN_DATA *pDomainVector = (DOMAIN_DATA *)pRespondData;
        if(errorCode != 0)
        {
            if(pDomainVector != NULL)
            {
                delete pDomainVector;
                pDomainVector = NULL;
            }
            Sleep(3000);
            if(g_autoLoginDlg != NULL)
            {
                initDomain();
            }
            break;
        }
        if( !m_domainList.empty()){
            m_domainList.clear();
        }
        if(pDomainVector!=NULL)
        {
            vector<string>::size_type i;
            for(i=0; i<pDomainVector->vstrDomainlists.size(); ++i)
            {
                string domain = pDomainVector->vstrDomainlists[i];
                m_domainList.append(QString(domain.c_str()));
            }
            delete pDomainVector;
        }
        emit autologinFinished(errorCode, dType);
        break;
    }
    case TYPE_LOGIN:
    case TYPE_AUTH_TOKEN:
    {
        LOGIN_DATA* pLoginData = (LOGIN_DATA*)pRespondData;
        if(errorCode != 0){
            if( pLoginData != NULL)
            {
                delete pLoginData;
                pLoginData = NULL;
            }
            Sleep(3000);
            if(g_autoLoginDlg != NULL){
                callGetUserName();
            }
            break;
        }
        if(NULL != pLoginData)
        {
            m_stNtAccountInfo = pLoginData->stLoginInfo;
            delete pLoginData;
        }
        emit autologinFinished(errorCode, dType);
        break;
    }
    case TYPE_LIST_USER_RES:
    {
        LIST_USER_RESOURCE_DATA* pListUserResData = (LIST_USER_RESOURCE_DATA*) pRespondData;
        if(!m_stAppBakList.empty()){
            m_stAppBakList.clear();
        }
        if(!m_stAppList.empty())
        {
            m_stAppList.clear();
        }
        if(NULL != pListUserResData)
        {
            m_stAppList = pListUserResData->stAppList;
            m_stAppBakList = pListUserResData->stAppBakList;
            delete pListUserResData;
        }
        if( errorCode != 0){
            logoutSession(BLOCKED);
            if(NULL != m_pSession)
            {
                m_pSession = NULL;
                CSession::Release();
            }
            Sleep(2000);
            initDomain();
            break;
        }
        if(m_stAppList.size() > 0)
        {
            emit autologinFinished(errorCode, dType);
            break;
        }else{
            Sleep(3000);
            if(g_autoLoginDlg != NULL){
                callListUserRes();
            }
        }
        break;
    }
    case TYPE_GETUSERINFO:
    {
        GET_USER_INFO_DATA* pGetUserInfoData = (GET_USER_INFO_DATA*)pRespondData;
        if(!m_vstVirtualDisks.empty()){
            m_vstVirtualDisks.clear();
        }
        if(NULL != pGetUserInfoData)
        {
            m_vstVirtualDisks = pGetUserInfoData->vstVirtualDisks;
            delete pGetUserInfoData;
        }
        emit autologinFinished(errorCode, dType);
        break;
    }
    case TYPE_START_DESKTOPPOOL:
    case TPPE_CHECK_DESKTOPPOOL_STATE:
    case TYPE_POWERON_DESKTOP :
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
    case TYPE_LOGOUT:
    {
        LogoutData *logout_data = (LogoutData *)pRespondData;
        if(logout_data!=NULL)
            delete logout_data;
        emit on_signal_logout_finished(errorCode, dType);
        break;
    }
    }
}

void AutoLoginDialog::processErrorCode(int errorCode, int opType)
{
    LOG_ERR("errcode:%d\t\t opType:%d", errorCode, opType);
    //fordib all messagebox
    showFullScreen();
    if(m_bLoginSession){
        logoutSession(BLOCKED);
    }
    if(NULL != m_pSession)
    {
        m_pSession = NULL;
        CSession::Release();
    }
    Sleep(1000);
    initDomain();
    return ;
//    setText(" ");
//    setmovieStop(true);
    switch(errorCode)
    {
    case ERROR_FAIL:
    {
        if(opType == TYPE_LOGIN)
            CMessageBox::CriticalBox(tr("Login failed"),this);
        else if( opType == TYPE_GET_USERNAME)
            CMessageBox::CriticalBox(tr("OccupyDesktop failed"), this);
        else if(opType == TYPE_LIST_USER_RES)
            CMessageBox::CriticalBox(tr("Get resource list failed"),this);
        else if(opType == TYPE_GETUSERINFO)
            CMessageBox::CriticalBox(tr("Get user information failed"),this);
        else if( opType == TYPE_GET_MONITORSINFO)
            CMessageBox::CriticalBox(tr("Get monitor information failed"),this);
        else if(opType == TYPE_LAUNCH_RES)
            CMessageBox::CriticalBox(tr("Connect to desktop failed"),this);
        else if(opType==TYPE_ATTACH_DISK || opType==TYPE_LAUNCH_ATTACH_DISK_FAILED)
            CMessageBox::CriticalBox(tr("Attach virtual disk failed"),this);
        else if(opType == TYPE_DESKTOP_NOT_AVAILABLE)
            CMessageBox::WarnBox(tr("No available desktop"),this);
        else
            CMessageBox::CriticalBox(tr("Opetate failed"),this);
        break;
    }
    default:
        cout << "will call showUiErrorCodeTip" << endl;
        showUiErrorCodeTip(errorCode);
        break;
    }
    //forbid all messagebox;
//    close();
}

void AutoLoginDialog::ipcClientProcessMsg(void *ipMsg)
{
	if(ipMsg == NULL){
	}
    QMutexLocker mutexLocker(&s_mutex);
    if(s_autoLoginDlg != NULL)
    {
        s_autoLoginDlg->logoutSession();
    }
}

int AutoLoginDialog::logoutSession(int flag /*= UNBLOCK*/)
{
    LOG_INFO("%s","before delete m_launchApp");
    LOG_INFO("%s","has delete m_launchApp");
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("%s", "new failed! NULL == pCall_param");
        return -1;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = AUTOLOGINDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = flag;
    param.taskUuid = taskUuid;
    m_pSession->logoutSession(param);
    return 0;
}


bool AutoLoginDialog::on_close_AutoLoginDialog()
{
    m_closeAutoLoginDialog = true;
    return m_closeAutoLoginDialog;
}

void AutoLoginDialog::on_local_connect()
{

}
void AutoLoginDialog::on_terminal_connect()
{

}

void AutoLoginDialog::setText(QString text)
{
    if(ui != NULL)
    {
        ui->label_text->setText(text);
    }
}
void AutoLoginDialog::setmovieStop(bool stop)
{
    if(stop)
    {
        m_movie->stop();
        ui->label_gif->setVisible(false);
    }
    else
    {
        ui->label_gif->setVisible(true);
        m_movie->start();
    }
}

void AutoLoginDialog::setPos(int x, int y, int w, int h)
{
    QRect rect = geometry();
    setGeometry(x, y, rect.width()>w?rect.width():w, rect.height()>h?rect.height():h);
}


void AutoLoginDialog::closeEvent(QCloseEvent *event)
{
    QMutexLocker mutexlocker(&s_mutex);
    if(NULL != m_launchApp)
    {
        CLaunchApp *launchApp = m_launchApp;
        m_launchApp = NULL;
        delete launchApp;//if the network is not good. it may spent a lot of time
    }
    if(m_bLoginSession)
    {
        logoutSession(BLOCKED);
    }
    if(NULL != m_pSession)
    {
        m_pSession = NULL;
        CSession::Release();
    }
    if(g_autoLoginDlg != NULL)
    {
        delete g_autoLoginDlg;
        g_autoLoginDlg = NULL;
    }
	QDialog::closeEvent(event);
}

void AutoLoginDialog::paintEvent(QPaintEvent *)
{
#ifdef VERSION_VCLASS
    ui->label_text->move(qApp->desktop()->width()*290/2880,
                             qApp->desktop()->height()*780/1080  -ui->label_text->height());

        ui->label_gif->move(qApp->desktop()->width()*290/2880 ,
                            qApp->desktop()->height()*840/1080 - ui->label_gif->height());
#else
        ui->label_gif->move(940*width()/m_bakPixmap.width() - width()/4,
                            1130*height()/m_bakPixmap.height());
        ui->label_text->move(940*width()/m_bakPixmap.width() - width()/4 ,
                             1130*height()/m_bakPixmap.height() + 40);
#endif
    QPainter paint(this);
    //    paint.setBrush(QBrush(m_backgroundPixmap));
    //    paint.drawRect(QRectF(-1, -1, width()+1, height()+1));
    paint.drawPixmap(rect(), m_backgroundPixmap);
}
void AutoLoginDialog::resizeEvent(QResizeEvent *)
{
    if(this->isFullScreen())
    {
        sysClose->setVisible(false);
        sysMax->setVisible(false);
        setWindowState(windowState() | Qt::WindowFullScreen);
    }
    if(isVisible())  // when restart vstation the AutoLoginDialog is !visible;
    {
        m_movie->setScaledSize(QSize(width()*1/4, 50*height()/m_bakPixmap.height()));
        //m_backgroundPixmap = QPixmap(IMAGE_FIRSTLOADING_BACKGROUND).scaled(qApp->desktop()->size());
        m_backgroundPixmap = QPixmap(vclient_image.background.data()).scaled(qApp->desktop()->size());
    }
}


void AutoLoginDialog::keyPressEvent(QKeyEvent * keyevent)
{
    /*
     * forbid Ctrl+q shortcut key to close;
     */
//    if((keyevent->modifiers() & Qt::ControlModifier) && keyevent->key() == Qt::Key_Q)
//        close();
    if(((keyevent->modifiers() & Qt::ControlModifier) && (keyevent->modifiers()& Qt::ShiftModifier)) && (keyevent->key() == Qt::Key_Return))
    {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        sysClose->setVisible(true);
        sysMax->setVisible(true);
    }

    //    switch(keyevent->key())
    //    {
    //    case Qt::Key_Escape:
    //    {
    //        setWindowState(windowState() & ~Qt::WindowFullScreen);
    //        sysClose->setVisible(true);
    //        sysMax->setVisible(true);
    //        break;
    //    }
    //    default:
    //        QDialog::keyPressEvent(keyevent);
    QDialog::keyPressEvent(keyevent);
}

void AutoLoginDialog::keyReleaseEvent(QKeyEvent *keyevent)
{
    //    switch(keyevent->key())
    //    {
    //    case Qt::Key_Escape:
    //    {
    //        m_escdown = false;
    //        break;
    //    }
    //    case Qt::CTRL:
    //    {
    //        m_ctrldown = false;
    //        break;
    //    }
    //    default:
    //        QDialog::keyReleaseEvent(keyevent);
    //    }
    QDialog::keyReleaseEvent(keyevent);
}

AutoLoginDialog::~AutoLoginDialog()
{
    delete ui;
}
