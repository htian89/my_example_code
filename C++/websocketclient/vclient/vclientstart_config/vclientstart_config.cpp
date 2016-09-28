#include "vclientstart_config.h"
#include "ui_vclientstart_config.h"
#include <QtGui/QLabel>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QPixmap>
#include <QtGui/QMainWindow>
#include <QtGui/QDesktopWidget>
#include <QFile>
#include <QRegExp>
#include <QRegExpValidator>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include "log.h"
#include <stdio.h>
#include <unistd.h>
#include <QGroupBox>


#define STYLE_SHEET_PUSHBTN "QPushButton{"\
    "border:1px groove #999999;"\
    "width: 70;"\
    "height: 25;"\
    "border-radius: 3px;"\
    "font: bold 15px;"\
    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FFFFFF, stop: 1 #e9e9e9)"\
    "}"\
    "QPushButton:default{"\
    "border:1px groove #999999;"\
    "border-radius: 3px;"\
    "font: bold 15px;"\
    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FFFFFF, stop: 1 #e9e9e9)"\
    "}"\
    "QPushButton:hover{"\
    "border:1px groove #999999;"\
    "border-radius: 3px;"\
    "font: bold 15px;"\
    "background-color: white;"\
    "}"\
    "QPushButton:pressed{"\
    "border:1px groove #999999;"\
    "border-radius: 3px;"\
    "font: bold 15px;"\
    "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #d9d9d9, stop: 1 #e9e9e9)"\
    "}"
#define CORNER_TOP_LEFT_IMG ":/image/resource/image/corner_left_top.png"
#define WINDOWS_ICON    ":image/resource/image/about_vCl_logo.png"
#define BACKGROUND_PIXMAP ":image/resource/image/skin_basic.png"

bool IsValidUrl(QString str)
{
    //    QRegExp rx("^(((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.)"
    //               "{3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])|([a-zA-Z0-9_\\-\\.])+\\."
    //               "(com|net|org|edu|int|mil|gov|arpa|biz|aero|name|coop|info|pro|museum|uk|me))"
    //               "((:[a-zA-Z0-9]*)?/?([a-zA-Z0-9\\-\\._\?\\,\'/\\\\+&amp;%\\$#\\=~])*)$");
//    QRegExp rx("^((((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])(([:/][a-zA-Z0-9\\-\\._\?\\,\'/\\\\+&amp;%\\$#\\=~]+))?)|(([a-zA-Z0-9_\\-\\.])+\\.(com|net|org|edu|int|mil|gov|arpa|biz|aero|name|coop|info|pro|museum|uk|me)(([:/][a-zA-Z0-9\\-\\._\?\\,\'/\\\\+&amp;%\\$#\\=~]+))?))$");
QRegExp rx("^(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])$");
    QRegExpValidator validator(rx,NULL);
    int pos = 0;
    if(validator.validate(str,pos) == QValidator::Acceptable)
        return true;
    return false;
}

VclientStart_Config::VclientStart_Config(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VclientStart_Config)
{
    ui->setupUi(this);


//    setAttribute(Qt::WA_TranslucentBackground);
//    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint|Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Startup config"));
    setWindowIcon(QIcon(WINDOWS_ICON));
//    m_backgroundPixmap.load(BACKGROUND_PIXMAP);

    ui->groupBox->setStyleSheet("QGroupBox{border: 1px solid #cccccc;}");
    ui->widget->setStyleSheet("QWidget{background-color: #e9e9e9}");





    QDialogButtonBox *requestButtonBox = new QDialogButtonBox();
    QPushButton *pushBtn_ok = new QPushButton(tr("OK"));
    QPushButton *pushBtn_cancel = new QPushButton(tr("Cancel"));
    pushBtn_ok->setFixedSize(70, 25);
    pushBtn_cancel->setFixedSize(70, 25);
    requestButtonBox->addButton(pushBtn_ok,QDialogButtonBox::AcceptRole);
    requestButtonBox->addButton(pushBtn_cancel,QDialogButtonBox::RejectRole);
    pushBtn_ok->setStyleSheet(STYLE_SHEET_PUSHBTN);
    pushBtn_cancel->setStyleSheet(STYLE_SHEET_PUSHBTN);
    ui->horizontalLayout_buttom->addStretch();
    ui->horizontalLayout_buttom->addWidget(requestButtonBox);

    connect(pushBtn_cancel, SIGNAL(clicked()),this, SLOT(close()));
    connect(pushBtn_ok, SIGNAL(clicked()), this, SLOT(on_pushBtn_ok_clieked()));

    ui->checkBox_manual->setDisabled(true);
//    ui->lineEdit_hostname->setDisabled(true);
    ui->lineEdit_ip->setDisabled(true);

//    ui->lineEdit_hostname->setStyleSheet("QLineEdit{background-color: white;}");
    ui->lineEdit_ip->setStyleSheet("QLineEdit{background-color: white;}");
    connect(ui->radioButton_auto, SIGNAL(clicked(bool)), ui->checkBox_manual, SLOT(setDisabled(bool)));
    connect(ui->radioButton_auto, SIGNAL(clicked(bool)), ui->lineEdit_ip, SLOT(setDisabled(bool)));
//    connect(ui->radioButton_auto, SIGNAL(clicked(bool)), ui->lineEdit_hostname, SLOT(setEnabled(bool)));

//    connect(ui->radioButton_local, SIGNAL(clicked(bool)), ui->lineEdit_hostname, SLOT(setDisabled(bool)));
    connect(ui->radioButton_local, SIGNAL(clicked(bool)), ui->checkBox_manual, SLOT(setEnabled(bool)));
    connect(ui->checkBox_manual, SIGNAL(clicked(bool)), ui->lineEdit_ip, SLOT(setEnabled(bool)));

    move((qApp->desktop()->width() - width())/2,
         (qApp->desktop()->height() - height())/2);


    initwidget();
}
int VclientStart_Config::initwidget()
{
    QFile file("/etc/vclient.conf");
    if(file.exists())
    {
        if(file.open(QIODevice::ReadOnly))
        {
            char buf[512] = {0};
            char hostname[512] = {0};
            int iRet = gethostname(hostname, 512);
            if(file.readLine(buf, 512) <= 0)
            {
                file.close();
                return -1;
            }else{
                file.close();
                if(buf[strlen(buf)-1] == '\n')
                {
                    buf[strlen(buf)- 1] = 0;
                }
                QString str;
                str = QString(buf).section("=", 1,1).section(",", 0,0);
                QString Ip;
                Ip = QString(buf).section("=", 2,2);
                QRegExp rx("^(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])$");

                QRegExpValidator validator(rx,NULL);
                int pos = 0;
                if(strstr(buf, "none") != NULL && strcmp("no", str.toUtf8().data()) == 0){
                    ui->radioButton_local->setChecked(true);
                    ui->checkBox_manual->setEnabled(true);
                }else if(strstr(buf, "none") != NULL && strcmp("yes", str.toUtf8().data()) == 0 && iRet == 0){
                       ui->radioButton_auto->setChecked(true);
//                       ui->lineEdit_hostname->setEnabled(true);
//                       ui->lineEdit_hostname->setText(hostname);
                }else if( validator.validate(Ip,pos) == QValidator::Acceptable && strcmp("yes", str.toUtf8().data()) == 0){
                    ui->radioButton_local->setChecked(true);
                    ui->checkBox_manual->setChecked(true);
                    ui->checkBox_manual->setEnabled(true);
                    ui->lineEdit_ip->setEnabled(true);
                    ui->lineEdit_ip->setText(Ip);
                }
                return -1;
            }
        }else{
            return -1;
        }
    }else{
        return -1;
    }
}

int VclientStart_Config::on_pushBtn_ok_clieked()
{
    char src[512] = {0};
    char hostname[512] = {0};
    if(ui->radioButton_auto->isChecked())
    {
        /*if(ui->lineEdit_hostname->text().isEmpty() || ui->lineEdit_hostname->text().length() > 50)
        {
            QMessageBox::warning(this, tr("Configure host name "), tr("host name error"), QMessageBox::Ok);
            return -1;
        }else{
            QString str = ui->lineEdit_hostname->text();
            strcpy(hostname, str.toUtf8().data());
            int iRet = sethostname(hostname, strlen(hostname));
            QFile file("/etc/hostname");
            if(file.exists())
            {
                if(file.open(QIODevice::WriteOnly))
                {
                    if(file.write(hostname) <= 0)
                    {
                        file.close();
                        iRet = -1;
                        LOG_ERR("Write fail");
                    }else{
                        file.close();
                        iRet = 0;
                    }
                }else{
                    LOG_ERR("Open error");
                    iRet = -1;
                }
            }else{
                iRet = -1;
            }
            if(iRet < 0)
            {
                QMessageBox::warning(this, tr("Configure host name "), tr("set host name error"), QMessageBox::Ok);
                return -1;
            }
        }*/
        strcpy(src, "launch=yes,launchip=none");
    }else if(ui->radioButton_local->isChecked())
    {
        if(ui->checkBox_manual->isChecked())
        {
            if(ui->lineEdit_ip->text().isEmpty() || !IsValidUrl(ui->lineEdit_ip->text()))
            {
                QMessageBox::warning(this, tr("Configure Server Ip"), tr("Server Ip error"), QMessageBox::Ok);
                return -1;
            }else{
                QString ip = ui->lineEdit_ip->text();
                QString str = "launch=yes,launchip=";
                str =str + ip;
                strcpy(src, str.toUtf8().data());
            }
        }else{
            strcpy(src, "launch=no,launchip=none");
        }
    }else{
        LOG_ERR("don't change the config");
        close();
        return -1;
    }
    int iRet = 0;
    QFile file("/etc/vclient.conf");
    if(file.exists())
    {
        if(file.open(QIODevice::WriteOnly))
        {
            if(file.write(src) <= 0)
            {
                file.close();
                iRet = -1;
                LOG_ERR("Write fail");
            }else{
                file.close();
                iRet = 0;
            }
        }else{
            LOG_ERR("Open error");
            iRet = -1;
        }
    }else{
        if(file.open(QIODevice::WriteOnly))
        {
            file.close();
            if(file.open(QIODevice::WriteOnly))
            {
                if(file.write(src) <= 0)
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
    }
    close();
    return iRet;
}



//void VclientStart_Config::mouseMoveEvent(QMouseEvent *_qm)
//{
//    if(_qm->buttons() == Qt::LeftButton && m_isMove)
//        move(_qm->globalPos() - m_pressPoint);
//}

//void VclientStart_Config::mousePressEvent(QMouseEvent *_qm)
//{
//    m_isMove = true;
//    m_pressPoint = _qm->pos();
//}

//void VclientStart_Config::mouseReleaseEvent(QMouseEvent *)
//{
//    m_isMove = false;
//}

//void VclientStart_Config::paintEvent(QPaintEvent *)
//{
//    QPainter paint(this);
//    paint.setRenderHint(QPainter::Antialiasing, true);
//    paint.setBrush(Qt::NoBrush);
//    QPen pen;
//    QColor color(0, 0, 0, 50);
//    paint.setBrush(QBrush(m_backgroundPixmap));
//    paint.setPen(Qt::transparent);
//    paint.drawRoundRect(QRectF(0,0,width(),height()),0,0);
//}

VclientStart_Config::~VclientStart_Config()
{
    delete ui;
}
