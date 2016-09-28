#include "openssl/ssl.h"
#include "ui/userlogindlg.h"
#include "ui/cclientapplication.h"
#include "../common/cconfiginfo.h"
#include "../common/filepath.h"
#include "../common/ds_session.h"
#include "../common/common.h"
#include "../common/ds_launchapp.h"
#include "ui/cmessagebox.h"
#include "ui/cupdate.h"
#include "common/log.h"
#include "../config.h"
#include "../ipc/ipcclient.h"
#include "../ipc/ipcitalc.h"
#include "../backend/globaldefine.h"
#include <QDir>
#include <QFileInfo>
#include <QDir>
#include <QSysInfo>
#include <networksettingdialog.h>
#include <autologindialog.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <a.out.h>
#include <netdb.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <fstream>
#include "vaccess.h"
#include "../imageconf.h"
#include <QTextCodec>


#include <iostream>
using namespace std;
#ifdef _WIN32
    #include <windows.h>
    #include <winsock2.h>
#endif

#define BUFSIZE 8192

char g_log_file_path[512];
int g_log_writeFileLevel = 0;
QString strAppPath;
bool gb_showCmdWindow = false;
bool gb_autostartVclient = true;
CConfigInfo* g_pConfigInfo = NULL;
IpcClient *g_ipcClient = NULL;  //Used to communication with websocket service
IpcItalc *g_ipcItalc = NULL;
CClientApplication* pApp = NULL;
AutoLoginDialog *g_autoLoginDlg= NULL;
bool g_autoLogin = false;
bool vclass_flag = false;
int role_dy = -1;
QTextCodec *codec = NULL;
int where_msg = -1;
CMessageBox *g_cmessagebox = NULL;
std::string version;

int WinSockStartup()
{
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(1, 1);
    err = WSAStartup(wVersionRequested, &wsaData);
    if(0!=err)
        LOG_ERR("WSAStartup failed.  %d",err);
#endif
    return 0;
}

int WinSockCleanup()
{
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

/* add by hansong 2013-7-11 -- start-- */
/*  solve #bug 4662 */

inline bool isStartedByVClientApp(const int argc, char *argv[])
{
    printf("%d, %s\n", argc, argv[0]);
#ifdef WIN32
    for(int i = 1; i < argc; i++)
    {
        if(NULL != strstr(argv[i], "--GAP"))
            return true;// VClient is  started by VClientApp
    }
    return false;//administrator privileges hasn't been authorized
#else
    return true;
#endif
}

/* add by hansong 2013-7-11 -- end--  */

/*int shareCtl( int argc, char* argv[])
{
    int shm_id;
    key_t key;
    st_setting *p_setting;
    shm_id = shmget(IPCKEY, 1024, 0640);
    if( shm_id != -1)
    {
        p_setting = (st_setting*)shmat(shm_id, NULL, 0);
        if( p_setting != (void*) -1)
        {
            shmdt(p_setting);
            shmctl(shm_id, IPC_RMID, 0);
        }
    }
    shm_id=shmget(IPCKEY,1028,0640|IPC_CREAT|IPC_EXCL);
    if(shm_id==-1)
    {
        printf("shmget error\n");
        return -1;
    }
    shm_id = shm_get(IPCKEY, 1024, 0640|IPC_CREAT|IPC_EXCL);
    strncpy(p_setting->agen,"zhangyl",10);
    printf( "agen:%s\n",p_setting->agen );

    p_setting->file_no = 1;
    p_setting=(st_setting*)shmat(shm_id,NULL,0);
    ****************
    if(shmdt(p_setting) == -1)
         perror(" detach error ");
}*/

int logInit()
{    
    strcpy(g_log_file_path,userPath.data());
    strcat(g_log_file_path, "log.log");
    //get log file size //now delete file when file>50M
    QFileInfo fileInfo(g_log_file_path);
    qint64 iFileSize = fileInfo.size();
    char writeMode[6]  = {0};
    strcpy(writeMode, "wb");
    if(iFileSize < 50*1024*1024)
    {
        strcpy(writeMode, "ab");
    }
    cout<<"log file write mode:\t"<<writeMode<<endl;

    FILE* fp = NULL;
    fp = fopen(g_log_file_path,writeMode);
    if(NULL == fp)
    {
        printf("fopen failed: file:%s\n", g_log_file_path);
        return -1;
    }
    fputs("log begin:\n", fp);
    fclose(fp);
#if 0
//get logLevel
    g_log_writeFileLevel = 3;
    char path_logLevel[MAX_LEN];
    strcpy(path_logLevel,userPath.data());
    strcat(path_logLevel, "loglevel.txt");

    FILE* fp_LogLevel = NULL;
//    fp_LogLevel = fopen(path_logLevel,"wb");
//        if(NULL == fp_LogLevel)
//            printf("fopen failed: file:%s\n", path_logLevel);
//        else
//        {
//            int iData = fputc('0', fp_LogLevel);
//            if( EOF == iData)
//            {
//                printf("fputc failed: file:%s\n", path_logLevel);
//            }
//            else
//                printf("fopen success: file:%s\n", path_logLevel);
//            fclose(fp_LogLevel);
//            //printf("filecontent(first char):%d\n", iData);
//        }
    fp_LogLevel = fopen(path_logLevel,"r");
    if(NULL == fp_LogLevel)
        printf("fopen failed: file:%s\n", path_logLevel);
    else
    {
        int iData = fgetc(fp_LogLevel);
        fclose(fp_LogLevel);
        printf("filecontent(first char):%d\n", iData);
        if(iData >= '0' && iData <= '9')
            g_log_writeFileLevel  = iData - '0';
    }
#endif
    LOG_INFO("LogLevel is: %d\n", g_log_writeFileLevel);
    return 0;
}

static int create_console()
{
#ifdef _WIN32
    if (!AllocConsole())
    {
        return 1;
    }
    freopen("CONOUT$", "w", stdout);
    printf("Debug console created.\n");
#endif
    return 0;
}

static int FindString(QStringList strList, QString str1, QString str2)
{
    int idx = -1;
    printf("find string :%s or %s \n", str1.toStdString().c_str(), str2.toStdString().c_str());
    for(int i = 0; i< strList.size(); i++)
    {
        if(0==strList[i].compare(str1) || (str2.length()>0&&0==strList[i].compare(str2)))
        {
            printf("got font:%s, index:%d\n", strList[i].toStdString().c_str(), i);
            idx = i;
            break;
        }
    }
    return idx;
}

int setFonts()
{
#ifdef WIN32
    QSysInfo::WinVersion ver =  QSysInfo::windowsVersion();
    if((int)ver < (int)QSysInfo::WV_VISTA)//it is xp
    {
        QFont appFont;
        appFont.setFamily(QString::fromLocal8Bit("����"));
        pApp->setFont(appFont);
        LOG_INFO("current font:%s", appFont.family().toUtf8().data());
        return 0;
    }
#endif
    printf("default font:%s\n", pApp->font().family().toStdString().c_str());
    QFont appFont;
    appFont.setPixelSize(12);
    QFontDatabase fontDb;
    QStringList fontNameList =  fontDb.families();

    int iRet = FindString(fontNameList, QString::fromLocal8Bit("΢���ź�"), "");//"Microsoft YaHei"
/*#ifdef WIN32
    QSysInfo::WinVersion ver =  QSysInfo::windowsVersion();
    if((int)ver < (int)QSysInfo::WV_VISTA)//it is xp
        iRet = -1;//in xp, yahei is not suitable
#endif*/
    if(iRet < 0)
    {
        iRet = FindString(fontNameList, QString::fromLocal8Bit("����"), "");//"SimHei"
        if(iRet < 0)
        {
            iRet = FindString(fontNameList, "Comic Sans MS", "");
            if(iRet < 0)
            {
                iRet = FindString(fontNameList, "Sans-serif", "");
            }
        }
    }
    if(iRet>=0)
    {
        printf("has found font:%s",fontNameList[iRet].toStdString().c_str());
        appFont.setFamily(fontNameList[iRet]);
        pApp->setFont(appFont);
    }
    printf("font2(set):%s\n", appFont.family().toStdString().c_str());
    printf("current font:%s\n", appFont.family().toStdString().c_str());
    LOG_INFO("current font:%s", appFont.family().toUtf8().data());
    return 0;
}

bool bLaunchVclient()
{
    QFile file("/etc/vclient.conf");
    if(file.exists())
    {
        char buf[512] = {0};
        if(file.open(QIODevice::ReadOnly))
        {
            if(file.readLine(buf, 512) <= 0)
            {
                file.close();
                return false;
            }else{
                printf("%s\n", buf);
                if(buf[strlen(buf)-1] == '\n')
                {
                    buf[strlen(buf)- 1] = 0;
                }
                QString str;
                str = QString(buf).section("=", 1,1).section(",", 0,0);
                if(strstr(buf, "none") != NULL && strcmp("no", str.toUtf8().data()) == 0){
                    file.close();
                    return true;
                }else{
                    file.close();
                    return false;
                }
            }
        }else{
            return false;
        }
    }else{
        return false;
    }
}

bool getGateway()
{
    QFile file("/etc/vclient.conf");
    if(file.exists())
    {
        char buf[512]={0};
        if(file.open(QIODevice::ReadOnly))
        {
            if(file.readLine(buf, 512) <= 0)
            {
                file.close();
                return false;
            }else{
                printf("%s\n", buf);
                if(buf[strlen(buf)-1] == '\n')
                {
                    buf[strlen(buf)- 1] = 0;
                }
                QString str;
                str = QString(buf).section("=", 1,1).section(",", 0,0);
                if(strstr(buf, "none") != NULL && strcmp("yes", str.toUtf8().data()) == 0){
                    file.close();
                    return true;
                }else{
                    file.close();
                    return false;
                }
             }
        }
    }else{
        return false;
    }
    return false;
}

bool getConfigIp(char ip[512])
{
    QFile file("/etc/vclient.conf");
    if(file.exists())
    {
        char buf[512] = {0};
        if(file.open(QIODevice::ReadOnly))
        {
            if(file.readLine(buf, 512) <= 0)
            {
                file.close();
                return false;
            }else{
                printf("%s\n", buf);
                if(buf[strlen(buf)-1] == '\n')
                {
                    cout << "++++++" << buf[strlen(buf)-1] << endl;
                    buf[strlen(buf)- 1] = 0;
                }
                QString str;
                str = QString(buf).section("=", 2,2);
                if(strstr(buf, "yes") != NULL && !str.isNull()){
                    printf("Ip: %s\n", str.toUtf8().data());
                    QRegExp rx("^(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])$");

                    QRegExpValidator validator(rx,NULL);
                    int pos = 0;
                    if(validator.validate(str,pos) == QValidator::Acceptable)
                    {
                        strcpy(ip, str.toUtf8().data());
                        file.close();
                        return true;
                    }else{
                        file.close();
                        return false;
                    }
                }else{
                    file.close();
                    return false;
                }
             }
        }
    }else{
        return false;
    }
    return false;
}

int getUuid(){
    QString uuidPath(userPath.data());
    uuidPath = uuidPath + "uuid";
    char value[512];
    memset(value, 0, 256);
    FILE *file = NULL;
    file = fopen(uuidPath.toUtf8().data(), "r");
    if(file == NULL)
    {
        QUuid uuid = QUuid::createUuid();
        QString string = uuid.toString();
        strcpy(value, string.toLocal8Bit().data());
        file = fopen(uuidPath.toUtf8().data(), "w");
        fprintf(file, "%s", value);
    }
    else
        if(NULL == fgets(value, 256, file)){
			LOG_ERR("%s", "get uuid file error");
		}
    fclose(file);

    if(NULL == g_pConfigInfo)
    {
        LOG_ERR("%s", "parameter error:NULL == g_pConfigInfo");
        return -1;
    }
    SETTINGS_LOGIN stLoginSettings;
    int iRet = g_pConfigInfo->getSettings_login(stLoginSettings);
    if(iRet < 0)
    {
        LOG_ERR("get config info from file failed. return value:%d", iRet);
        memset(&stLoginSettings, 0, sizeof(SETTINGS_LOGIN));
        return -6;
    }
    if(strcmp(stLoginSettings.stUserInfo.uuid, value) != 0){
        strcpy(stLoginSettings.stUserInfo.uuid, value);
        g_pConfigInfo->setSettings_login(stLoginSettings);
    }
    return 0;
}

#include <sys/wait.h>
static void sigchld_handler(int signo)
{
    waitpid(-1, NULL, WNOHANG);
}

void getvClientVersion(std::string &version)
{
    ifstream fin(VERSION_FILE_DIR.data());
    if(fin.is_open())
    {
        char c[32]={'\0'};
        fin.read(c, 32);
        version = std::string(c);
        int len = version.size();
        if(len) {
            if (version.at(len - 1) == '\n' || version.at(len - 1) == '\r')
                version.replace(len - 1, 1, "\0");
        }
        fin.close();
    }
    else
    {
        LOG_ERR("Open file %s failed!\n", VERSION_FILE_DIR.data());
    }
}

void setvClientImages(std::string &version)
{
    /* general picture */
    vclient_image.vertical_line = GENERAL_DIR + VERTICAL_LINE_IMG;
    vclient_image.corner_left_top = GENERAL_DIR + CORNER_LEFT_TOP_IMG;

    /* background picture, anyone is different*/
    vclient_image.background = IMAG_DIR + version + "/" + BACKGROUND_IMG;

    if (version.find("vclass") != std::string::npos)
    {
        vclass_flag = true;
        vclient_image.firstloading = JSJQ_DIR + FIRSTLOADING_IMG; //autologin Progress bar
        vclient_image.Fronview_configure_title = GENERAL_VCLASS_DIR + FRONVIEW_CONFIGURE_TITLE_IMG; //vClass set interface
        vclient_image.fronview_login_background = GENERAL_VCLASS_DIR + FRONVIEW_LOGIN_BACKGROUND_IMG; //vClass login interface
        vclient_image.about_vCl_logo = GENERAL_DIR + ABOUT_VCL_LOGO_IMG;
        vclient_image.icon_close =  GENERAL_VCLASS_DIR + ICON_CLOSE;
        vclient_image.icon_minimize = GENERAL_VCLASS_DIR + ICON_MINIMIZE;
        if (version.find("LiV") != std::string::npos)
        {
            // Liv_vclass
            vclient_image.Fronview_configure_title = IMAG_DIR + version + "/" + FRONVIEW_CONFIGURE_TITLE_IMG;
            vclient_image.fronview_login_background = IMAG_DIR + version + "/" + FRONVIEW_LOGIN_BACKGROUND_IMG;
        }
    } else {
        vclient_image.Fronview_configure_title = IMAG_DIR + version + "/" + FRONVIEW_CONFIGURE_TITLE_IMG;
        vclient_image.fronview_login_background = IMAG_DIR + version + "/" + FRONVIEW_LOGIN_BACKGROUND_IMG;
        vclient_image.about_vCl_logo = GENERAL_DIR + ABOUT_VCL_LOGO_IMG;
        vclient_image.firstloading = GENERAL_DIR + FIRSTLOADING_IMG;
        vclient_image.icon_close =  GENERAL_DIR + ICON_CLOSE;
        vclient_image.icon_minimize = GENERAL_DIR + ICON_MINIMIZE;
        if (version == "JSJQ")
        {
            vclient_image.about_vCl_logo = JSJQ_DIR + ABOUT_VCL_LOGO_IMG;
            vclient_image.jsjq_title = JSJQ_DIR + JSJQ_TITLE_IMG;
            vclient_image.logo_jsjq = JSJQ_DIR + LOGO_JSJQ_IMG;
            vclient_image.firstloading = JSJQ_DIR + FIRSTLOADING_IMG;
            vclient_image.icon_close =  JSJQ_DIR+ ICON_CLOSE;
            vclient_image.icon_minimize = JSJQ_DIR + ICON_MINIMIZE;
        }
        else if (version == "DHC")
        {
            vclient_image.about_vCl_logo = IMAG_DIR + version + "/" + ABOUT_VCL_LOGO_IMG;
        } else {
        }
    }
}

int main(int argc, char *argv[])
{
    pApp = new CClientApplication(argc, argv);//CClientApplication app(argc, argv);
#if 1
     codec = QTextCodec::codecForName("utf8");
#endif
    WinSockStartup();
    setvbuf(stdout, (char *)NULL, _IONBF, 0);

    QDir dir(QString(userPath.data()));
    if(!dir.exists())
        dir.mkdir(QString(userPath.data()));

    signal(SIGCHLD,  sigchld_handler);
    strAppPath = pApp->applicationDirPath();
    QDir::setCurrent(strAppPath);//set current dir
    logInit();//init log

    //std::string version;
    getvClientVersion(version);
    setvClientImages(version);
    LOG_INFO("version is %s\n", version.c_str());
    LOG_INFO("current dir:%s, setting dir:%s", QDir::currentPath().toUtf8().data(), strAppPath.toUtf8().data());
    if(argc>1)
        LOG_INFO("argv[1]:%s", argv[1]);
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-v")==0 || strcmp(argv[i], "--version")==0)
        {
            cout<<VER1<<"."<<VER2<<"."<<VER3<<"Build"<<VER4<<endl;
            return 0;
        }
        if(strstr(argv[i], "--getuserpath")!=0)
        {
            logInit();
            saveUserPath();
            return 0;
        }
        if( strstr(argv[i], "--uninstallupdateservice")!=0)
        {
            return CUpdate::setAutoUpdate(false);
        }
        if(strstr(argv[i], "--stopupdateservice")!=0)
        {
            return CUpdate::stopAutoUpdateService();
        }
        if(strcmp(argv[i], "--with-debug") == 0)//disable stdout buffer
        {
            create_console();
            gb_showCmdWindow = true;
        }
        if(strstr(argv[i], "--notautostartvclient") != NULL)
        {
            gb_autostartVclient = false;
        }
    }

    QString currentWorkDir = QDir::currentPath();
    qDebug() << "Current work dir:   " << currentWorkDir;

#ifdef WIN32   /* add by hansong 2013-7-11 -- start--    #bug 4662*/
   if(!isStartedByVClientApp(argc,argv))
    {
        LOG_INFO("%s", "##### debug : isStartedByVClientApp  called");
        QStringList paraList;
        for(int i = 1; i < argc; i++)
        {
            paraList.append(argv[i]);
        }
       if( QProcess::startDetached("vClient_app.exe", paraList) )
        {
           Sleep(5000);
            pApp->quit();// start vClient_app succeed then quit
        }
        else
        {
            LOG_ERR("%s", "QProcess::startDetached failed.");
            Sleep(5000);
        }
        return 0;
    }
#endif /* add by hansong 2013-7-11 -- end--  */
    //set translator
    QTranslator translator,qt_translator;
    QLocale locale = QLocale::system();//used to get local system language
    if(locale.language() == QLocale::Chinese)
    {
        bool b = translator.load("vclient_zh");
        LOG_INFO("load vclient_zh return value:%d", int(b));
        qt_translator.load("qt_zh_CN");
        pApp->installTranslator(&translator);
        pApp->installTranslator(&qt_translator);
    }
    //Set fonts
    setFonts();

    //make sure only one vclient is Running
    QSharedMemory *sharedMemory = new QSharedMemory(QString("SingleInstanceIdenfity"));
    volatile short i = 2;
    while(i--)
    {
        if (sharedMemory->attach(QSharedMemory::ReadOnly))
            sharedMemory->detach();
        LOG_INFO("sharedMemoryError: %d", sharedMemory->error());
    }
    if (!(sharedMemory->create(1)))
    {
        CMessageBox::CriticalBox(QObject::tr("An instance is running"), NULL);
        return 0;
    }
    g_cmessagebox = new CMessageBox;
    SSL_library_init();
//    SSL_load_error_strings();
//    OpenSSL_add_all_algorithms();

    //Register the type to make it enable to be a parameter between signal and slot
    qRegisterMetaType<LIST_USER_RESOURCE_DATA>("LIST_USER_RESOURCE_DATA");
    qRegisterMetaType<GET_USER_INFO_DATA>("GET_USER_INFO_DATA");
    qRegisterMetaType<DOMAIN_DATA>("DOMAIN_DATA");
    qRegisterMetaType<GET_RESOURCE_PARAMETER>("GET_RESOURCE_PARAMETER");
    qRegisterMetaType<LAUNCH_DESKTOP_DATA>("LAUNCH_DESKTOP_DATA");
    qRegisterMetaType<APP_LIST>("APP_LIST");
    qRegisterMetaType<LAUNCH_TYPE>("LAUNCH_TYPE");

    //load config from file
    qDebug() << "User path: "<< userPath;
    g_pConfigInfo = new CConfigInfo((QString(userPath.data())+"config.xml").toStdString().data());

    CUpdate* pupdate = NULL;
    SETTINGS_VCLIENT stVclientSetting;
    memset(&stVclientSetting, 0, sizeof(SETTINGS_VCLIENT));
    stVclientSetting.m_updateSetting = UNKNOWN_UPDATING_STATUS;
    int iRet = g_pConfigInfo->getSettings_vclient(stVclientSetting);
    if(iRet < 0)
    {
        LOG_ERR("getSettings_vclient failed. return value:%d", iRet);
    }
    else
    {
        LOG_INFO("stVclientSetting.m_updateSetting:%d", stVclientSetting.m_updateSetting);
        if(AUTO_DETECT == stVclientSetting.m_updateSetting)
        {//check whether has new version of vclient.
            pupdate = new CUpdate(stVclientSetting);
            pupdate->start();
        }
        else if(AUTO_UPDATE == stVclientSetting.m_updateSetting)
        {//start auto dectect service

        }
    }

//    g_ipcItalc = new IpcItalc;
    g_ipcClient = new IpcClient;
    THREAD_HANDLE pHandle;
    CThread::createThread((THREAD_HANDLE *)(&pHandle), NULL, (FUN_THREAD)(&ServerTeacherForFap), NULL);
//    if(stVclientSetting.iAutoConnectToServer == 0)
//    {
//        NetWorkSettingDialog* pNetSetDlg = new NetWorkSettingDialog;
//        pNetSetDlg->show();
//    }
//    else
//    {
//        UserLoginDlg *userLoginDlg = new UserLoginDlg;
//        userLoginDlg->show();
//    }
    //get uuid for every type login
    cout <<"wo de uuid shi : " <<  iRet  << endl;
    bool bStartVclient = true;
    char Ip[512] = {0};
//    char gateWay[512] = {0};
    if(getConfigIp(Ip)){
        cout << "Ip+++" << Ip << endl;
        strcpy(stVclientSetting.m_network.stFirstServer.serverAddress, Ip);
        strcpy(stVclientSetting.m_network.stFirstServer.port, "80");
        stVclientSetting.m_network.stFirstServer.isHttps = 0;
        strcpy(stVclientSetting.m_network.stPresentServer.serverAddress, Ip);
        strcpy(stVclientSetting.m_network.stPresentServer.port, "80");
        stVclientSetting.m_network.stPresentServer.isHttps = 0;
        g_pConfigInfo->setSettings_vclient(stVclientSetting);
        if(stVclientSetting.iAutoConnectToServer == 0)
        {
            NetWorkSettingDialog* pNetSetDlg = new NetWorkSettingDialog;
            pNetSetDlg->show();
        }
        else
        {
            UserLoginDlg *userLoginDlg = new UserLoginDlg(true, true, 0, true);
            userLoginDlg->show();
        }
    }else if(getGateway()){
        AutoLoginDialog *pAutoLoginDlg = new AutoLoginDialog();
        g_autoLoginDlg = pAutoLoginDlg;
        pAutoLoginDlg->show();
        pApp->setQuitOnLastWindowClosed(false);
    }else if(bLaunchVclient()&&gb_autostartVclient){
        LOG_INFO("don't login vclient!");
        cout << "dont login vclient" << endl;
        bStartVclient  = false;
    }else{
        if(stVclientSetting.iAutoConnectToServer == 0)
        {
            NetWorkSettingDialog* pNetSetDlg = new NetWorkSettingDialog;
            pNetSetDlg->show();
        }
        else
        {
            UserLoginDlg *userLoginDlg = new UserLoginDlg(true, true);
            userLoginDlg->show();
        }
    }
    //pApp->setQuitOnLastWindowClosed(false);
    int ret;
    if(bStartVclient)
    {
        ret = pApp->exec();
    }
    if (sharedMemory->isAttached())
        sharedMemory->detach();
    delete sharedMemory;
    if(NULL != pupdate)
        delete pupdate;
    pupdate = NULL;
    close_debug();
    if(g_ipcClient != NULL)
        delete g_ipcClient;
    if(g_ipcItalc != NULL)
        delete g_ipcItalc;
    WinSockCleanup();
    return ret;
}
