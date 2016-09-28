#include <QtGui/QApplication>
#include <QTranslator>
#include "updateinfoview.h"
#include "globaldefine.h"
#include "tinyxml/tinyxml.h"
#include "log.h"
#include "autoupdate.h"
#include <QDebug>
#include <string.h>


//Log file
char g_log_file_path[512];
int g_log_writeFileLevel = 0;
bool g_bAutoUpate;
bool g_bAutoMode;
char g_sourceIp[MAX_LEN];

void getNetworkInfo(NETWORKINFO &networkInfo)
{
    TiXmlDocument doc(networkPath.data());
    memset(&networkInfo, 0, sizeof(NETWORKINFO));
    int ret = doc.LoadFile();
    if(ret < 0)
        return ;

    TiXmlNode *pXmlVclientSetting = doc.FirstChild("VClientSettings");
    if(pXmlVclientSetting==NULL)
        return;

    TiXmlElement* pNetworkSettings = pXmlVclientSetting->FirstChildElement("NetworkSettings");
    if(NULL == pNetworkSettings)
    {
        LOG_ERR("%s", "network not configed");
        return;
    }
    TiXmlElement* pPresentServer = pNetworkSettings->FirstChildElement("presentServer");
    if(NULL != pPresentServer)
    {
        const char* pData = pPresentServer->GetText();
        if(NULL != pData)
            strcpy(networkInfo.ipAddress, pData);
    }

    TiXmlElement *pPort = pNetworkSettings->FirstChildElement("port");
    if(NULL != pPort)
    {
        const char* pData = pPort->GetText();
        if(NULL != pData)
            strcpy(networkInfo.port, pData);
    }
}

int logInit()
{
    strcpy(g_log_file_path,logPath.data());
    FILE* fp = NULL;
    fp = fopen(g_log_file_path,"w");
    if(NULL == fp)
    {
        printf("fopen failed: file:%s\n", g_log_file_path);
        return -1;
    }
//    LOG_INFO("log begin:");
    fputs("log begin:\n", fp);
    fclose(fp);
#if 0
//get logLevel
    g_log_writeFileLevel = 2;
    char path_logLevel[MAX_LEN];
    strcpy(path_logLevel,userPath.data());
    strcat(path_logLevel, "loglevel.txt");
    FILE* fp_LogLevel = NULL;
    fp_LogLevel = fopen(path_logLevel,"rb");
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
    printf("g_log_writeFileLevel:%d\n", g_log_writeFileLevel);
#endif

    return 0;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList strlist = app.arguments();

    g_bAutoUpate = false;
    g_bAutoMode = false; // autoupdate mode;
    memset(g_sourceIp, 0, sizeof(g_sourceIp));
    foreach (QString str, strlist) {
        if(str.contains("--autoupdate")){
            g_bAutoUpate = true;
        }
        if(str.contains("--SourceIp")){
            strcpy(g_sourceIp,str.section('=',1,1).toUtf8().data());
            qDebug() << "ther sourceIp :" << g_sourceIp;
        }
        if(str.contains("--automode")){
            g_bAutoMode = true; // administrator maual update;
        }
    }
    QTranslator translate;
    translate.load("update_zh");
    app.installTranslator(&translate);

    logInit();

    NETWORKINFO networkInfo;
    getNetworkInfo(networkInfo);
    AutoUpdate *update = NULL;
    if(g_bAutoUpate)
    {
        if(strlen(g_sourceIp) <= 0)
            update = new AutoUpdate(networkInfo.ipAddress, networkInfo.port);
        else
            update = new AutoUpdate(g_sourceIp, networkInfo.port);
    }
    else
    {
        UpdateInfoView *updateView = new UpdateInfoView(networkInfo.ipAddress, networkInfo.port);
        updateView->show();
    }
    int iRet = app.exec();
    close_debug();
    if(update != NULL)
    {
        delete update;
    }
    return iRet;
}
