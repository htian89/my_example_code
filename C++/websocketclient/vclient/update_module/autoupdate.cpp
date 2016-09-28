#include "autoupdate.h"

#include "log.h"
#include "cthread.h"
#include "globaldefine.h"
#include <QFile>
#include <QProcess>
#include <QApplication>
#include <QIcon>
#include "qmessagebox.h"

extern bool g_bAutoMode;
static double s_updatedMod = 0.0;
static double s_totalMod = 0.0;

AutoUpdate::AutoUpdate(const char *serverAdress, const char *port, QObject *parent):
    QObject(parent),
    m_serverAddress(serverAdress),
    m_port(port),
    m_udpSocket(NULL),
    m_updateResult(UNFINISHED),
    m_rebootResult(UNKNOWN)
{
    m_iTimeout = 30;

    m_timer.setInterval(1000);
    QString title = tr("Tip");
    m_text += tr("auto update finished, reboot count down:");
    m_text += QString::number(m_iTimeout) ;
    m_text += tr("second");
    m_msgBox = new QMessageBox(QMessageBox::Question,title, m_text);
    m_msgBox->setWindowFlags(Qt::WindowStaysOnTopHint);
    m_cancelBtn = m_msgBox->addButton(tr("Cancel"), QMessageBox::NoRole);
    m_rebootBtn = m_msgBox->addButton(tr("Reboot"), QMessageBox::NoRole);
    connect(m_rebootBtn, SIGNAL(clicked()), this,SLOT(on_slot_reboot_clicked()));
    connect(m_cancelBtn, SIGNAL(clicked()), this, SLOT(on_slot_cancel_clicked()));
    connect(&m_timer,SIGNAL(timeout()), this, SLOT(on_slot_operate_reboot()) );
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
	int iRet = 0;
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
        if(s_updatedMod == s_totalMod && s_totalMod > 0){
            LOG_INFO("update successed! please reboot the system to make the modifiton available.");
            if(g_bAutoMode){
                m_timer.start();
                m_msgBox->show();
                LOG_INFO("%s","SHOW MESSAGEBOX");
            }else{
                iRet = system("reboot");
				LOG_INFO("system(reboot) return value: %d", iRet);
                qApp->quit();
            }
        }
    }
        break;
    }
}

void AutoUpdate::on_slot_reboot_clicked()
{
    int iRet;
    if(m_msgBox == NULL){
        return;
    }
    m_rebootResult = REBOOTOK;
    m_msgBox->close();
    if( m_iTimeout >= 0){
        m_timer.stop();
        iRet = system("reboot");
        LOG_INFO("system(reboot) return value: %d", iRet);
        qApp->quit();
    }
}

void AutoUpdate::on_slot_cancel_clicked(){
    m_rebootResult = REBOOTNO;
    if(m_msgBox == NULL){
        return;
    }
    m_msgBox->close();
    if( m_iTimeout >= 0){
        m_timer.stop();
        LOG_INFO("%s","update finished, don't reboot");
        qApp->quit();
    }
}

void AutoUpdate::on_slot_operate_reboot()
{
    int iRet;
    m_iTimeout--;
    m_text.clear();
    m_text += tr("auto update finished, reboot count down:");
    m_text += QString::number(m_iTimeout) ;
    m_text += tr("second");
    if(m_msgBox == NULL){
        return;
    }
    m_msgBox->setTextFormat(Qt::RichText);
    m_msgBox->setText(m_text);
    LOG_INFO("%s","set the msg text");
    if(m_iTimeout == 0 ){
        m_timer.stop();
        if(m_msgBox != NULL){
            m_msgBox->close();
            m_msgBox = NULL;
        }
        iRet = system("reboot");
        LOG_INFO("system(reboot) return value: %d", iRet);
        qApp->quit();
    }
}

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

    qDebug() << "update the progress bar";
    qDebug() << "s_totalMod:" << s_totalMod;
    qDebug() << "s_updatedMod:" << s_updatedMod;

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
