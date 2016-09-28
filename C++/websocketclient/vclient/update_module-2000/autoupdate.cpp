#include "autoupdate.h"

#include "log.h"
#include "cthread.h"
#include "globaldefine.h"
#include <QFile>
#include <QProcess>
#include <QApplication>
#include <QMessageBox>
#include "qmessagebox.h"
#include <unistd.h>
#include <time.h>
#include <stdio.h>

extern bool g_bAutoMode;
static double s_updatedMod = 0.0;
static double s_totalMod = 0.0;

AutoUpdate::AutoUpdate(const char *serverAdress, const char *port, QObject *parent):
    QObject(parent),
    m_serverAddress(serverAdress),
    m_port(port),
    m_udpSocket(NULL),
    m_updateResult(UNFINISHED)
{
    checkVersion();
}

int AutoUpdate::compareVersion(QString currentStr, QString newStr)
{
    if(newStr.isEmpty() || currentStr.isEmpty())
        return -2;
    if(newStr == currentStr)
        return 0;
    int currnetIndex = currentStr.indexOf(".");
    int newIndex = newStr.indexOf(".");
    if(currnetIndex == -1 || newIndex == -1)
    {
        int c_firstNumPosition = currentStr.indexOf("B");
        int n_firstNumPosition = newStr.indexOf("B");
        if (currentStr.left(c_firstNumPosition).toInt() < newStr.left(n_firstNumPosition).toInt())
            return 1;
        else if (currentStr.left(c_firstNumPosition).toInt() > newStr.left(n_firstNumPosition).toInt())
            return -1;
        int c_lastNum = currentStr.mid(6,4).toInt();
        int n_lastNum = newStr.mid(6,4).toInt();
        if (c_lastNum < n_lastNum)
            return 1;
        else
            return 0;
    }
    QString leftCurrentStr = currentStr.left(currnetIndex);
    QString leftNewStr= newStr.left(newIndex);
    int leftCurrentNum = leftCurrentStr.toInt();
    int leftNewNum = leftNewStr.toInt();
    if(leftNewNum > leftCurrentNum)
        return 1;
    else if(leftNewNum < leftCurrentNum)
        return -1;
    else
    {
        QString rightCurrentStr = currentStr.right(currentStr.length()-currnetIndex-1);
        QString rightNewStr = newStr.right(newStr.length()- newIndex - 1);
        return compareVersion(rightCurrentStr.toUtf8(),rightNewStr.toUtf8());
    }

}

void AutoUpdate::on_checkversion_finished(QString version)
{
    if(version.length() == 0){
        LOG_ERR("check version == NULL");
        qApp->quit();
        return;
    }
    QByteArray localVersion;
    QFile *file = new QFile(versionFile.data());
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    localVersion = file->readLine();
    file->close();
    delete file;

    localVersion.remove(localVersion.size()-1, 1);

    int ret = compareVersion(QString(localVersion), version);
    if(ret <= 0){
        LOG_INFO("this is the last version");
        qApp->quit();
        return ;
    }else{
        LOG_INFO("has new verison");
    }
    if(m_udpSocket == NULL)
    {
        m_udpSocket = new MyUdpSocket(m_serverAddress);
        connect(m_udpSocket, SIGNAL(sendDatagramsToView(QByteArray)), this, SLOT(showOnTextArea(QByteArray)));
    }
    else
    {
        m_udpSocket->startServer();
    }
}

void AutoUpdate::showOnTextArea(QByteArray recvDatagrams)
{
//    qUpdateDebug("show the params...");
    QString datagrams(QString::fromLocal8Bit(recvDatagrams.data()));
    QStringList spData = datagrams.split("\n");
    qDebug() << spData;

    update_progress(spData);

    processUPdateResult(m_updateResult, datagrams);
}

void AutoUpdate::processUPdateResult(UPDATERESULT updateResult,const QString &datagrams)
{
    switch(updateResult)
    {
    case -1:
    {
        if(datagrams.contains(QString::fromLocal8Bit("升级本身有更新")))
        {
            m_updateResult = UNFINISHED;
            m_udpSocket->startServer();
        }
        else
        {
            LOG_ERR("update failed");
            qApp->quit();
        }
    }
        break;
    case 0:
        break;
    case 1:
    {
        //if(s_updatedMod == s_totalMod && s_totalMod > 0){
        if (m_updateResult == FINISHED){
            LOG_INFO("update successed! please reboot the system to make the modifiton available.");
            if(g_bAutoMode){
                QMessageBox msgBox;
                msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
                if( QMessageBox::Ok == msgBox.question(NULL, tr("Tip"), tr("auto update finished, reboot ?"), QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Ok) ){
                    system("reboot");
                }else{
                    LOG_INFO("%s","update finished, don't reboot");
                }
            }else{
                system("reboot");
            }
            qApp->quit();
        }
    }
        break;
    }
}
#if 0
void AutoUpdate::update_progress(const QStringList &spData)
{
    int i, j = 0;
    for(i=0; i<spData.size(); ++i)
    {
        qDebug() << QString("spData[%1]").arg(QString::number(i)) << spData.at(i);
        if(spData.at(i).contains("Totalmod"))
        {
            j++;
            int index = spData.at(i).indexOf(":");
            QString total  = spData.at(i).mid(index+1);
            s_totalMod = total.toInt();
        }
        if(spData.at(i).contains("Updatedmod"))
        {
            j++;
            int index = spData.at(i).indexOf(":");
            QString updated = spData.at(i).mid(index+1);
            s_updatedMod = updated.toInt();
        }
        if(spData.at(i).contains("Finish"))
        {
            j++;
            int index = spData.at(i).indexOf(":");
            QString result = spData.at(i).mid(index+1);
            if(result == "failed")
                m_updateResult = FAILED;
            else if(result == "successed")
                m_updateResult = FINISHED;
        }
        if(j == 3)
            break;
    }


//    if(s_totalMod != 0)
//    {
//        double rate = s_updatedMod/s_totalMod;
//        int value = 100*rate;
//        if(rate == 1.0)
//            ui->progressBar->setValue(98);
//        else
//            ui->progressBar->setValue(value);
//    }
}
#else
void AutoUpdate::update_progress(const QStringList &spData)
{
    int i, j = 0, bar = 0;
    char process[64];

    srand(time(NULL));
    for(i = 0; i < spData.size(); ++i)
    {
        if(spData.at(i).contains("success"))
        {
            if(spData.at(i).contains("success1"))
                bar =  10 + rand() % 20;
            else if(spData.at(i).contains("success2"))
                bar =  30 + rand() % 20;
            else if(spData.at(i).contains("success3"))
                bar =  70 + rand() % 20;

            memset(process, 0, 64);
            strcpy(process, "已完成 ");
            sprintf(process + strlen(process), " %2d%s", bar, "%");
            LOG_INFO("%s", process);
            j++;
        }
        else if(spData.at(i).contains("finished"))
        {
            j++;
            int index = spData.at(i).indexOf(":");
            QString result = spData.at(i).mid(index+1);
            if(result == "failed")
                m_updateResult = FAILED;
            else if(result == "finished")
                m_updateResult = FINISHED;
            LOG_INFO("已完成 100%");
        }
        else
        {
            QString output = spData.at(i);
            QByteArray ba = output.toLatin1();
            LOG_INFO("%s", ba.data());
            j++;
        }

        if(j == 3)
            break;
    }
}
#endif
void AutoUpdate::checkVersion()
{
    CThread *thread= new CThread(m_serverAddress, atoi(m_port));
    connect(thread,SIGNAL(on_signal_complete(QString)) , this, SLOT(on_checkversion_finished(QString)));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

AutoUpdate::~AutoUpdate()
{
    if( m_udpSocket != NULL){
        delete m_udpSocket;
        m_udpSocket = NULL;
    }
}
