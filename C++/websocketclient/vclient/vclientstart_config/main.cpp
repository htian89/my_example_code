#include <QtGui/QApplication>
#include "vclientstart_config.h"
#include <QTranslator>
#include "log.h"
#include "globaldefine.h"
#include <QDebug>
#include <QSharedMemory>
#include <QMessageBox>


char g_log_file_path[512];
int g_log_writeFileLevel;

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
    return 0;
}
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translate;
    translate.load("vclientstartconfig_zh");
    app.installTranslator(&translate);
    QSharedMemory *sharedMemory = new QSharedMemory(QString("Vclientstartconfig"));
    volatile short i = 2;
    while(i--)
    {
        if (sharedMemory->attach(QSharedMemory::ReadOnly))
            sharedMemory->detach();
        LOG_INFO("sharedMemoryError: %d", sharedMemory->error());
    }
    if (!(sharedMemory->create(1)))
    {
        QMessageBox::warning(NULL, QObject::tr("Warning"), QObject::tr("An instance is running"), QMessageBox::Ok);
        return 0;
    }

    logInit();
    VclientStart_Config *dlg = new VclientStart_Config;
    dlg->show();

    int iRet = app.exec();
    if (sharedMemory->isAttached())
        sharedMemory->detach();
    delete sharedMemory;
    return iRet ;
}
