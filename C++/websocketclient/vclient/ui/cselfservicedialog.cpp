#include "cselfservicedialog.h"
#include "ui_cselfservicedialog.h"
#include "sysbutton.h"
#include "../common/log.h"
#include <QPainter>
#include <QBitmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

CSelfServiceDialog::CSelfServiceDialog(SELF_SERVICE_STATUS serviceStatus, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSelfServiceDialog),
    m_serviceStatus(serviceStatus)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Popup);
    //setAttribute(Qt::WA_DeleteOnClose);

//    setStyleSheet("QDialog{"
//                  "border:1px solid #e9e9e9;"
//                  "border-radius:50px;"
//                  "}");

    pixmap = new QPixmap;
//    pixmap->fill(QColor(0xe9, 0xe9, 0xe9));//load(":image/resource/image/selfservice_background.png", 0, Qt::AvoidDither | Qt::ThresholdDither | Qt::ThresholdAlphaDither);
    pixmap->load(":image/resource/image/skin_basic.png", 0, Qt::AvoidDither | Qt::ThresholdDither | Qt::ThresholdAlphaDither);
//    QPalette pal = palette();
//    pal.setBrush(QPalette::Window, QBrush(*pixmap));
//    pal.setBrush(QPalette::WindowText, QBrush(QRgb(0x99cc00)));
//    setPalette(pal);

    m_unitStart = new CSelfServiceUnit("icon_button_link_unable.png", tr("Disconnect"), tr("Disconnect"));//"icon_button_start.png", tr("Start"), tr("Start")
    m_unitReboot = new CSelfServiceUnit("icon_button_restart.png", tr("Restart"), tr("Restart"));
    m_unitShutdown= new CSelfServiceUnit("icon_button_shutdown.png", tr("Shutdown"), tr("Shutdown"));
    connect(m_unitStart, SIGNAL(clicked()), this, SLOT(on_btnStart_clicked()));
    connect(m_unitReboot, SIGNAL(clicked()), this, SLOT(on_btnRestart_clicked()));
    connect(m_unitShutdown, SIGNAL(clicked()), this, SLOT(on_btnClose_clicked()));

    m_btnLayout = new QHBoxLayout;
    m_btnLayout->setContentsMargins(20, 8, 20, 8);//(50, 22, 0, 5);
    m_btnLayout->addWidget(m_unitStart);
    m_btnLayout->addWidget(m_unitReboot);
    m_btnLayout->addWidget(m_unitShutdown);

    setLayout(m_btnLayout);
    resize(sizeHint());//220 60
    setSelfServiceStatus(m_serviceStatus);
}

void CSelfServiceDialog::setSelfServiceStatus(SELF_SERVICE_STATUS status)
{
    qDebug()<<"setSelfServiceStatus:"<<status;
    //std::cout<<"CSelfServiceDialog::setSelfServiceStatus(SERVICE_STATUS status) has called. status:"<<status<<std::endl;
    m_serviceStatus = status;

    setConnectBtn(m_serviceStatus);
    switch(m_serviceStatus)
    {
    case ALLOWCLOSE:
        setEnabled_ex(false, true, true);
        break;
    case ALLOWSTART:
        setEnabled_ex(false, false, true);
        break;
    case ALLOW_CONNECT:
        setEnabled_ex(true, true, true);
        break;
    case CANNOT_CONNECT:
        setEnabled_ex(false, true, true);
        break;
    case ALLOW_DISCONN:
        setEnabled_ex(true, true, true);
        break;
    case CANNOT_DISCONN:
        setEnabled_ex(false, true, true);
        break;
    default:
        setEnabled_ex(false, false, false);
        break;
    }
}

void CSelfServiceDialog::setEnabled_ex(bool bStartEnabled, bool bRebootEnbled, bool bShutdownEnabled)
{
    qDebug()<<"setEnabled_ex:"<<bStartEnabled<<bRebootEnbled<<bShutdownEnabled;
    LOG_INFO("bStartEnabled %d, bRebootEnbled %d, bShutdownEnabled %d", int(bStartEnabled), int(bRebootEnbled), int(bShutdownEnabled));
    if(NULL != m_unitStart)
        m_unitStart->setEnabled_ex(bStartEnabled);
    else
        LOG_ERR("%s", "NULL == m_unitStart");

    if(NULL != m_unitReboot)
        m_unitReboot->setEnabled_ex(bRebootEnbled);
    else
        LOG_ERR("%s", "NULL == m_unitReboot");

    if(NULL != m_unitShutdown)
        m_unitShutdown->setEnabled_ex(bShutdownEnabled);
    else
        LOG_ERR("%s", "NULL == m_unitShutdown");
}

int CSelfServiceDialog::setConnectBtn(SELF_SERVICE_STATUS status)
{
    switch(status)
    {
    case ALLOWCLOSE:
        m_unitShutdown->setContents("icon_button_shutdown.png", tr("Shutdown"), tr("Shutdown"));
        break;
    case ALLOWSTART:
        m_unitShutdown->setContents("icon_button_shutdown.png", tr("Start"), tr("Start"));
        break;
    case ALLOW_CONNECT:
    case CANNOT_CONNECT:
        m_unitStart->setContents("icon_button_link.png", tr("Connect"), tr("Connect"));
        m_unitShutdown->setContents("icon_button_shutdown.png", tr("Shutdown"), tr("Shutdown"));
        break;
    case ALLOW_DISCONN:
    case CANNOT_DISCONN:
        m_unitStart->setContents("icon_button_link_unable.png", tr("Disconnect"), tr("Disconnect"));
        m_unitShutdown->setContents("icon_button_shutdown.png", tr("Shutdown"), tr("Shutdown"));
        break;
    default:
        break;//do nothing
    }
    return 0;
}

CSelfServiceDialog::~CSelfServiceDialog()
{
    delete ui;
    if(pixmap!=NULL)
        delete pixmap;
}

void CSelfServiceDialog::paintEvent(QPaintEvent *_pe)
{
//    pixmap = new QPixmap;
//    pixmap->fill(QColor(0xe9, 0xe9, 0xe9));//load(":image/resource/image/selfservice_background.png", 0, Qt::AvoidDither | Qt::ThresholdDither | Qt::ThresholdAlphaDither);
//    pixmap->load(":image/resource/image/skin_basic.png", 0, Qt::AvoidDither | Qt::ThresholdDither | Qt::ThresholdAlphaDither);
    QPalette pal = palette();
    pal.setBrush(QPalette::Window, QBrush(*pixmap));
    pal.setBrush(QPalette::WindowText, QBrush(QRgb(0x99cc00)));
    setPalette(pal);
//    QPainter paint(this);
//    paint.setBrush(QColor(0xe9, 0xe9, 0xe9));
//    paint.setPen(QColor(0xe9, 0xe9, 0xe9));
//    paint.drawRoundRect(rect(), 4, 4);
	QDialog::paintEvent(_pe);
}

void CSelfServiceDialog::resizeEvent(QResizeEvent*)
{
    //qDebug()<<"CSelfServiceDialog:: +++++++++++++++++resizeEvent hascalled";
//    QBitmap b(size());
//    b.clear();
//    QPainter paint_bitmap(&b);
//    paint_bitmap.setBrush( Qt::color1 );
//    paint_bitmap.setPen( Qt::color1 );
//    paint_bitmap.drawRoundRect(0, 0, width()-1, height()-1, 4, 4);
//    setMask(b);
}

void CSelfServiceDialog::closeEvent(QCloseEvent *_ce)
{
    emit on_signal_rotatePixmap();
    QDialog::closeEvent(_ce);
}

void CSelfServiceDialog::on_btnStart_clicked()
{
    //if(btnSatrt->isEnabled())//do not response to click m_labelStart(when the label is disabled)
    {
        emit on_signal_start();
//        hide();
        close();
    }
}

void CSelfServiceDialog::on_btnRestart_clicked()
{
    //if(btnReboot->isEnabled())
    {
        emit on_signal_restart();
//        hide();
        close();
    }
}

void CSelfServiceDialog::on_btnClose_clicked()
{
    //if(btnShutdown->isEnabled())
    {
        int iIsStart = 0; //0 :shutdown 1 start
        if(m_serviceStatus == ALLOWSTART)
            iIsStart = 1;
        emit on_signal_shutdown(iIsStart);
//        hide();
        close();
    }
}


CLabel::CLabel(const QString &text, QWidget *parent) :
    QLabel(text, parent)
{
    m_bEnabled = false;
    QPalette pal = palette();
    pal.setColor(QPalette::WindowText, QColor(QRgb(0x669900)));
    setPalette(pal);
    setFixedSize(sizeHint());
}

void CLabel::mousePressEvent(QMouseEvent *)
{
    emit clicked();
}

void CLabel::enterEvent(QEvent *)
{
    if(m_bEnabled)
    {
        QPalette pal = palette();
        pal.setColor(QPalette::WindowText, QColor(QRgb(0x77b200)));
        setPalette(pal);
    }
}

void CLabel::leaveEvent(QEvent *)
{
    setEnabled_ex(m_bEnabled);
}

void CLabel::setEnabled_ex(bool enable)
{
    m_bEnabled = enable;
    QPalette pal = palette();
    if(!enable)
    {
        pal.setColor(QPalette::WindowText, QColor(QRgb(0x598501)));//89 133 1 -->59 85 1  //0xffffff
        setPalette(pal);
    }
    else
    {
        pal.setColor(QPalette::WindowText, QColor(QRgb(0x669900)));
        setPalette(pal);
    }
}

CSelfServiceUnit::CSelfServiceUnit(QString picName, QString tipText, QString labelText, QWidget *parent/* = 0*/):
    QWidget(parent),m_label(labelText, this),m_labelImg(this)//,m_btn(this)
{
//   m_labelImg.setFixedSize(m_labelImg.sizeHint());
//    m_label.setFixedSize(m_label.sizeHint());
    setContents(picName, tipText, labelText);

    QVBoxLayout *Layout = new QVBoxLayout;
    Layout->setContentsMargins(20,6,20,6);
    Layout->addWidget(&m_labelImg);//(&m_btn);
    Layout->addWidget(&m_label);
    setLayout(Layout);

    setFixedSize(sizeHint());

    m_bEnabled = false;
}

void CSelfServiceUnit::setEnabled_ex(bool enable)
{
    m_bEnabled = enable;
    if(!m_bEnabled)
    {
        QPalette palLabel = palette();
        palLabel.setColor(QPalette::WindowText, QColor(QRgb(0xb5b5b5)));//QColor(QRgb(0xffffff)));
        m_label.setPalette(palLabel);

        m_labelImg.setPixmap(pixmap.copy(imgWidth*3, 0, imgWidth, imgHeight));
    }
    else
    {
        QPalette palLabel = palette();
        palLabel.setColor(QPalette::WindowText, QColor(QRgb(0x598501)));//0x669900
        m_label.setPalette(palLabel);

        m_labelImg.setPixmap(pixmap.copy(0, 0, imgWidth, imgHeight));
    }
}

int CSelfServiceUnit::setContents(QString picName, QString tipText, QString labelText)
{
    pixmap.load(":image/resource/image/"+picName);
    imgWidth = pixmap.width()/4;
    imgHeight = pixmap.height();
    m_labelImg.resize(pixmap.size());
    m_labelImg.setPixmap(pixmap.copy(0, 0, imgWidth, imgHeight));
    m_labelImg.setAlignment(Qt::AlignCenter);

    m_label.setText(labelText);
    m_label.setAlignment(Qt::AlignCenter);
    if( !tipText.isEmpty()){
        //setToolTip(tipText);
    }
    return 0;
}

void CSelfServiceUnit::enterEvent(QEvent *)
{
    if(m_bEnabled)
    {
        QPalette palLabel = palette();
        palLabel.setColor(QPalette::WindowText, QColor(QRgb(0x9fee1d)));//0xc6f578
        m_label.setPalette(palLabel);

        m_labelImg.setPixmap(pixmap.copy(imgWidth, 0, imgWidth, imgHeight));
    }
}

void CSelfServiceUnit::leaveEvent(QEvent *)
{
    setEnabled_ex(m_bEnabled);
}

void CSelfServiceUnit::mousePressEvent(QMouseEvent *)
{
    if(m_bEnabled)//only send the clicked signal when the widget is in enabled status
        emit clicked();
}
