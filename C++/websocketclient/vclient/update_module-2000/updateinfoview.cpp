#include "updateinfoview.h"
#include "ui_updateinfoview.h"
#include "cthread.h"
#include "globaldefine.h"
#include "log.h"
#include "chttp.h"
#include <QFile>
#include <QMessageBox>
#include <QDesktopWidget>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

static double s_updatedMod = 0.0;
static double s_totalMod = 0.0;
extern THREAD_TYPE g_threadType;

UpdateInfoView::UpdateInfoView(const char *serverAddr, const char *port, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UpdateInfoView)
{
    ui->setupUi(this);
    if(serverAddr != NULL)
        ui->lineEdit_ip->setText(QString(serverAddr));
    if(port != NULL)
        ui->lineEdit_port->setText(QString(port));
    //    system("killall vClient");
    setWindowIcon(QIcon(":image/update_moudle.png"));
    udpSocket = NULL;
    m_updateResult = UNFINISHED;
    move((qApp->desktop()->width() - width())/2,
         (qApp->desktop()->height() - height())/2);

    ui->pushButton_reboot->setEnabled(false);
}

UpdateInfoView::~UpdateInfoView()
{
    delete ui;
    if(udpSocket != NULL)
        delete udpSocket;
}

int  UpdateInfoView::checkVersion()
{
    //Check the version
    //    std::string farClientVersion;
    if(ui->lineEdit_ip->text().isEmpty() || ui->lineEdit_port->text().isEmpty())
    {
        ui->pushButton_update->setEnabled(true);
        QMessageBox::critical(this, tr("critical"), tr("server address is empty, please input an address"));
        return -1;
    }
    appendText(tr("It is checking the vesion information from server....."));
    g_threadType = TYPE_CHECKVERSION;
    CThread *thread = new CThread(ui->lineEdit_ip->text(), ui->lineEdit_port->text().toInt());
    connect(thread, SIGNAL(on_signal_complete(QString)), this, SLOT(on_checkVersion_finish(QString)));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();

    return 0;
}

void UpdateInfoView::on_checkVersion_finish(QString version)
{
    if(version.length() == 0)
    {
        ui->pushButton_update->setEnabled(true);
        setInsertTextColor(QColor(255,0,0));
        appendText(tr("connect server failed..."));
        setInsertTextColor(QColor(0,0,0));
        return ;
    }

    QByteArray localVersion;
    QFile *file = new QFile(versionFile.data());
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    localVersion = file->readLine();
    file->close();
    delete file;

    localVersion.remove(localVersion.size()-1, 1);

    int ret = compareVersion(QString(localVersion), version);
    if(ret <= 0)
    {
        appendText(tr("This is the lastest version"));
        ui->pushButton_update->setEnabled(true);
        return;
    }
    else
        appendText(tr("Find the new version is ") + version + "\n");

    appendText(tr("connecting to server..."));
    /*
     * thread func: getclientinfo()
     * desc: get the install package path and name;
     */
    g_threadType = TYPE_GETCLIENTINFO;
    CThread *thread = new CThread(ui->lineEdit_ip->text(), ui->lineEdit_port->text().toInt());
    connect(thread, SIGNAL(on_signal_getClientInfo_finished(QString,int, int)), this, SLOT(on_slot_getClientInfo_finished(QString,int,int)));
    connect(thread, SIGNAL(on_signal_download_finished(int)), this, SLOT(on_slot_download_finished(int)));
    connect(thread, SIGNAL(on_signal_thread_download_progress(const char *, double)), this, SLOT(on_slot_download_progress(const char *, double)));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}
/**
 * @brief UpdateInfoView::on_slot_getClientInfo_finished
 * @param filename:the install package name
 * @param filesize: the install package size, now is ignore
 * @param errorcode:most is http error;
 */
void UpdateInfoView::on_slot_getClientInfo_finished(QString filename,int filesize, int errorcode)
{
    if(errorcode < 0){
        setInsertTextColor(QColor(255,0,0));
        appendText(tr("connect server failed..."));
        setInsertTextColor(QColor(0,0,0));
        ui->pushButton_update->setEnabled(true);
        return;
    }
    appendText(tr("download install package ..."));
}
/**
 * @brief UpdateInfoView::on_slot_download_finished
 * @param errorcode: the http error or operate file error;
 * @desc finished download, now upgrade;
 */
void UpdateInfoView::on_slot_download_finished(int errorcode)
{
    if(errorcode < 0)
    {
        setInsertTextColor(QColor(255,0,0));
        appendText(tr("download install package failed..."));
        setInsertTextColor(QColor(0,0,0));
        ui->pushButton_update->setEnabled(true);
        ui->progressBar->setValue(0);
        return;
    }
    appendText(tr("download install package success."));
    LOG_INFO("download install package success.");
    ui->progressBar->setValue(0);

    if(udpSocket == NULL)
    {
        LOG_INFO("udpSocket == NULL");
        udpSocket = new MyUdpSocket(ui->lineEdit_ip->text().toUtf8());
        connect(udpSocket, SIGNAL(sendDatagramsToView(QByteArray)), this, SLOT(showOnTextArea(QByteArray)));
    }
    else
    {
        LOG_INFO("udpSocket != NULL");
        ui->plainTextEdit->clear();
        udpSocket->startServer();
    }
}
/**
 * @brief UpdateInfoView::on_slot_download_progress
 * @param progress: the down package progress;
 * @desc just update ui;
 */
void UpdateInfoView::on_slot_download_progress(const char *monitor, double progress)
{
    if(strlen(monitor) > 0)
    {
        appendText(monitor);
    }
    ui->progressBar->setValue(int(progress*100));
}

int UpdateInfoView::compareVersion(QString currentStr, QString newStr)
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

void UpdateInfoView::on_pushButton_update_clicked()
{
    ui->plainTextEdit->clear();
    ui->pushButton_update->setEnabled(false);
    if(checkVersion() < 0)
        return;

}

void UpdateInfoView::showOnTextArea(QByteArray recvDatagrams)
{
    QString datagrams(QString::fromLocal8Bit(recvDatagrams.data()));
    QStringList spData = datagrams.split("\n");

    update_progress(spData);   

    //appendText(datagrams);
    processUPdateResult(m_updateResult, datagrams);
}

void UpdateInfoView::update_progress(const QStringList &spData)
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
            appendText(QString::fromUtf8(process));
            ui->progressBar->setValue(bar);
            //sleep(1);
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
            appendText(QString::fromUtf8("已完成 100%"));
            ui->progressBar->setValue(100);
        }
        else
        {
            QString output = spData.at(i);
            appendText(output);
        }

        if(j == 3)
            break;
    }
}

void UpdateInfoView::appendText(const QString &text)
{
    ui->plainTextEdit->appendPlainText(text);
}

void UpdateInfoView::processUPdateResult(UPDATERESULT updateResult,const QString &datagrams)
{
    //    qUpdateDebug("process the updateResult...");
    switch(updateResult)
    {
    case -1:
    {
        if(datagrams.contains(QString::fromLocal8Bit("升级本身有更新")))
        {
            m_updateResult = UNFINISHED;
            udpSocket->startServer();
        }

        else
        {
            setInsertTextColor(QColor(255,0,0));
            appendText(tr("update failed"));
            setInsertTextColor(QColor(0,0,0));
            //            ui->pushButton_update->setEnabled(true);
        }
    }
        break;
    case 0:
        break;
    case 1:
    {
        LOG_INFO("get successed");
        //if(s_updatedMod == s_totalMod && s_totalMod > 0)
        if (m_updateResult == FINISHED)
        {
            ui->progressBar->setValue(100);
            setInsertTextColor(QColor(0,255,0));
            appendText(tr("update successed! please reboot the system to make the modifiton available."));
            setInsertTextColor(QColor(0,0,0));
            ui->pushButton_reboot->setEnabled(true);
        }
    }
        break;
    }
}

void UpdateInfoView::setInsertTextColor(QColor color)
{
    QTextCharFormat fmt;
    fmt.setForeground(color);
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    cursor.mergeCharFormat(fmt);
    ui->plainTextEdit->mergeCurrentCharFormat(fmt);
}

void UpdateInfoView::on_pushButton_reboot_clicked()
{
    system("reboot");
}
