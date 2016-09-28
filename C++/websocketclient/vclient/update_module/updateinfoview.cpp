#include "updateinfoview.h"
#include "ui_updateinfoview.h"
#include "cthread.h"
#include "globaldefine.h"
#include "log.h"
#include <QFile>
#include <QMessageBox>
#include <QDesktopWidget>

static double s_updatedMod = 0.0;
static double s_totalMod = 0.0;

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
    if(ret <= 0)
    {
        appendText(tr("This is the lastest version"));
        ui->pushButton_update->setEnabled(true);
        return;
    }
    else
        appendText(tr("Find the new version is ") + version + "\n");

    appendText(tr("connecting to server..."));
    LOG_INFO("connecting to server...");

    if(udpSocket == NULL)
    {
        udpSocket = new MyUdpSocket(ui->lineEdit_ip->text().toUtf8());
        connect(udpSocket, SIGNAL(sendDatagramsToView(QByteArray)), this, SLOT(showOnTextArea(QByteArray)));
    }
    else
    {
        ui->plainTextEdit->clear();
        udpSocket->startServer();
    }
}

int UpdateInfoView::compareVersion(QString currentStr, QString newStr)
{
    if(newStr.isEmpty() || currentStr.isEmpty())
        return -2;
    QByteArray cur_ver, new_ver;
    cur_ver = currentStr.toLatin1();
    new_ver = newStr.toLatin1();
    LOG_INFO("\ncurrent version: %s\nnew version: %s\n", cur_ver.data(), new_ver.data());

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
//    qUpdateDebug("show the params...");
    QString datagrams(QString::fromLocal8Bit(recvDatagrams.data()));
    QStringList spData = datagrams.split("\n");

    update_progress(spData);

    appendText(datagrams);
    processUPdateResult(m_updateResult, datagrams);
}

void UpdateInfoView::update_progress(const QStringList &spData)
{
    int i, j = 0;
    for(i=0; i<spData.size(); ++i)
    {
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

    if(s_totalMod != 0)
    {
        double rate = s_updatedMod/s_totalMod;
        int value = 100*rate;
        if(rate == 1.0)
            ui->progressBar->setValue(98);
        else
            ui->progressBar->setValue(value);
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
        if(s_updatedMod == s_totalMod && s_totalMod > 0)
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
	int iRet =  system("reboot");
	LOG_INFO("system(reboot) return : %d", iRet);
}
