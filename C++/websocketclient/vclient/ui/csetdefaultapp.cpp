#include <QPainter>
#include <QBitmap>
#include <QEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include "csetdefaultapp.h"
#include "../config.h"
#include "../common/log.h"

CSetDefaultApp::CSetDefaultApp(QObject *parent/* = 0*/):
    QWidgetAction(parent)//,
    //QWidget(NULL)
{
    m_setDefaultAppWidget = NULL;
    //m_setDefaultAppWidget = new CSetDefalutAppWidget();
    //setDefaultWidget(m_setDefaultAppWidget);
}
CSetDefaultApp::~CSetDefaultApp()
{
    if(NULL != m_setDefaultAppWidget)
    {
        delete m_setDefaultAppWidget;
        m_setDefaultAppWidget = NULL;
    }
}

QWidget* CSetDefaultApp::createWidget(QWidget * parent)
{
    //qDebug()<<"++++++createWidget has called...........";
    if(NULL == m_setDefaultAppWidget)
    {
        m_setDefaultAppWidget = new CSetDefalutAppWidget(this, parent);
        setDefaultWidget(m_setDefaultAppWidget);
    }
    m_setDefaultAppWidget->setParent(parent);
    return m_setDefaultAppWidget;
}

void CSetDefaultApp::deleteWidget(QWidget *)
{
    //qDebug()<<"++++++deleteWidget has called...........";
    if(NULL != m_setDefaultAppWidget)
        m_setDefaultAppWidget->hide();
}

int CSetDefaultApp::getSetDefaultAppInfo(LAUNCH_TYPE& launchType, bool& bIsOkClicked)
{
    if(NULL == m_setDefaultAppWidget)
    {
        LOG_ERR("%s","NULL==m_setDefaultAppWidget");
        return -1;
    }
    bIsOkClicked = m_setDefaultAppWidget->isOkClicked();
    return m_setDefaultAppWidget->getLaunchType(launchType);

}

void CSetDefaultApp::setAppSupportProtocol(SUPPORT_PROTOCAL suppProtocol)
{
    if(NULL == m_setDefaultAppWidget)
    {
        LOG_ERR("%s","NULL==m_setDefaultAppWidget");
        return ;
    }
    m_setDefaultAppWidget->setAppSupportProtocol(suppProtocol);
}

////////////////////////////////////////////////////////////////////
CSetDefalutAppWidget::CSetDefalutAppWidget(CSetDefaultApp* pSetAppAction, QWidget *parent/* = 0*/):
    QWidget(parent)
{
    m_bIsOkClicked = false;
    m_pSetAppAction = pSetAppAction;
//set windows attr | flags
    setWindowFlags(Qt::Popup);
//    QString styleSheet = "CSetDefalutAppWidget{"
//            "border:2px solid #e9e9e9;"
//            "border-radius:40px;}";
//    setStyleSheet(styleSheet);

//add a label
    m_labelMsg = new QLabel(QObject::tr("Default connecting type:"));
//add two radiobtn
    m_radioBtn_RDP = new QRadioButton(QObject::tr("RDP connect"));
#ifdef VERSION_VSAP
    m_radioBtn_FAP = new QRadioButton(QObject::tr("VSAP connect"));
#else
    m_radioBtn_FAP = new QRadioButton(QObject::tr("FAP connect"));
#endif
    m_radioBtn_TERMINAL = new QRadioButton(QObject::tr("TERMINAL connect"));

    m_radioBtn_RDP->setStyleSheet(STYLE_SHEET_RADIOBTN);
    m_radioBtn_FAP->setStyleSheet(STYLE_SHEET_RADIOBTN);
    m_radioBtn_TERMINAL->setStyleSheet(STYLE_SHEET_RADIOBTN);
    m_radioBtn_RDP->setChecked(true);
//add btn
    m_dlgBox = new QDialogButtonBox();
    m_btn_ok = new QPushButton(tr("OK"));
    m_btn_cancel = new QPushButton(tr("Cancel"));
    m_btn_ok->setStyleSheet(STYLE_SHEET_PUSHBTN);
    m_btn_cancel->setStyleSheet(STYLE_SHEET_PUSHBTN);
    m_btn_ok->setFixedSize(66,26);
    m_btn_cancel->setFixedSize(66,26);
    m_dlgBox->addButton(m_btn_ok, QDialogButtonBox::AcceptRole);
    m_dlgBox->addButton(m_btn_cancel, QDialogButtonBox::RejectRole);
    connect(m_dlgBox, SIGNAL(accepted()), this, SLOT(on_okBtn_clicked()));
    connect(m_dlgBox, SIGNAL(rejected()), this, SLOT(on_cancelBtn_clicked()));
//set layout
    //m_hboxLayout = new QHBoxLayout();
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->addWidget(m_labelMsg);
    m_mainLayout->addWidget(m_radioBtn_RDP);
    m_mainLayout->addWidget(m_radioBtn_FAP);
#ifndef _WIN32
    m_mainLayout->addWidget(m_radioBtn_TERMINAL);
#endif
    m_mainLayout->addWidget(m_dlgBox);
    setLayout(m_mainLayout);
}

CSetDefalutAppWidget::~CSetDefalutAppWidget()
{
    qDebug()<<"++++++~CSetDefalutAppWidget has called...........";
    if(NULL != m_labelMsg)
        delete m_labelMsg;
    m_labelMsg = NULL;

    if(NULL != m_radioBtn_RDP)
        delete m_radioBtn_RDP;
    m_radioBtn_RDP = NULL;

    if(NULL != m_radioBtn_FAP)
        delete m_radioBtn_FAP;
    m_radioBtn_FAP = NULL;

    if (NULL != m_radioBtn_TERMINAL)
        delete m_radioBtn_TERMINAL;
    m_radioBtn_TERMINAL = NULL;

    if(NULL != m_dlgBox)
        delete m_dlgBox;
    m_dlgBox = NULL;
}

void CSetDefalutAppWidget::on_okBtn_clicked()
{
    m_bIsOkClicked = true;
    if(NULL != m_pSetAppAction)
        m_pSetAppAction->trigger();//activate(QAction::Trigger);
    else
    {
        LOG_ERR("%s", "NULL == m_pSetAppAction");
    }    
}
void CSetDefalutAppWidget::on_cancelBtn_clicked()
{
    m_bIsOkClicked = false;
    if(NULL != m_pSetAppAction)
        m_pSetAppAction->trigger();//activate(QAction::Trigger);
    else
    {
        LOG_ERR("%s", "NULL == m_pSetAppAction");
    }    
}

void CSetDefalutAppWidget::setAppSupportProtocol(SUPPORT_PROTOCAL suppProtocol)
{//NO_SUPPORT=-1, BOTH_SUPPORT, ONLY_RDP, ONLY_FAP, ONLY_TERMINAL
    if(NO_SUPPORT == suppProtocol)
    {
        m_radioBtn_FAP->setEnabled(false);
        m_radioBtn_RDP->setEnabled(false);
        m_radioBtn_TERMINAL->setEnabled(false);
        m_radioBtn_FAP->setVisible(false);
        m_radioBtn_RDP->setVisible(false);
        m_radioBtn_TERMINAL->setVisible(false);
    }
    else if(BOTH_SUPPORT == suppProtocol)
    {
        m_radioBtn_FAP->setEnabled(true);
        m_radioBtn_RDP->setEnabled(true);
        m_radioBtn_FAP->setChecked(true);
        m_radioBtn_TERMINAL->setEnabled(false);
        m_radioBtn_TERMINAL->setVisible(false);
    }
    else if(ONLY_RDP == suppProtocol)
    {
        m_radioBtn_FAP->setEnabled(false);
        m_radioBtn_RDP->setEnabled(true);
        m_radioBtn_RDP->setChecked(true);
        m_radioBtn_TERMINAL->setEnabled(false);
        m_radioBtn_TERMINAL->setVisible(false);
        m_radioBtn_FAP->setVisible(false);
    }
    else if(ONLY_FAP == suppProtocol)
    {
        m_radioBtn_FAP->setEnabled(true);
        m_radioBtn_RDP->setEnabled(false);
        m_radioBtn_FAP->setChecked(true);
        m_radioBtn_TERMINAL->setEnabled(false);
        m_radioBtn_TERMINAL->setVisible(false);
        m_radioBtn_RDP->setVisible(false);
    }
    else if(ONLY_TERMINAL == suppProtocol)
    {
        m_radioBtn_FAP->setEnabled(false);
        m_radioBtn_RDP->setEnabled(false);
        m_radioBtn_TERMINAL->setEnabled(true);
        m_radioBtn_TERMINAL->setChecked(true);
        m_radioBtn_RDP->setVisible(false);
        m_radioBtn_FAP->setVisible(false);
    }
    else
    {
        m_radioBtn_FAP->setEnabled(false);
        m_radioBtn_RDP->setEnabled(false);
        m_radioBtn_TERMINAL->setEnabled(false);
        m_radioBtn_FAP->setVisible(false);
        m_radioBtn_RDP->setVisible(false);
        m_radioBtn_TERMINAL->setVisible(false);
    }
}

int CSetDefalutAppWidget::getLaunchType(LAUNCH_TYPE &launchType)
{
    if(NULL==m_radioBtn_RDP || NULL==m_radioBtn_FAP || NULL==m_radioBtn_TERMINAL)
    {
        LOG_ERR("NULL==m_radioBtn_RDP || NULL==m_radioBtn_FAP  %p, %p", m_radioBtn_RDP, m_radioBtn_FAP, m_radioBtn_TERMINAL);
        return -1;
    }
    int iRet = 0;
    if(m_radioBtn_RDP->isChecked() && m_radioBtn_RDP->isEnabled())
        launchType = LAUNCH_TYPE_RDP;
    else if(m_radioBtn_FAP->isChecked() && m_radioBtn_FAP->isEnabled())
        launchType = LAUNCH_TYPE_FAP;
    else if(m_radioBtn_TERMINAL->isChecked() && m_radioBtn_TERMINAL->isEnabled())
            launchType = LAUNCH_TYPE_TERMINAL;
    else
        iRet = -1;
    return iRet;
}


void CSetDefalutAppWidget::paintEvent(QPaintEvent*)
{
//    qDebug()<<"CSelfServiceDialog:: +++++++++++++++++paintEvent hascalled";
//    QBitmap b(size());
//    b.clear();
//    QPainter paint_bitmap(&b);
//    paint_bitmap.setBrush( Qt::color1 );
//    paint_bitmap.setPen( Qt::color1 );
//    paint_bitmap.drawRoundRect(0, 0, width()-1, height()-1, 40, 40);
//    setMask(b);

    QPainter paint(this);
    paint.setBrush(QColor(0xe9, 0xe9, 0xe9));
    paint.setPen(QColor(0xe9, 0xe9, 0xe9));
//    paint.setBrush(QColor(0, 0, 0));
//    paint.setPen(QColor(0, 0, 0));
    paint.drawRoundRect(rect(), 40, 40);


}

void CSetDefalutAppWidget::resizeEvent(QResizeEvent*)
{
    qDebug()<<"CSetDefalutAppWidget:: +++++++++++++++++resizeEvent hascalled";
    QBitmap b(size());
    b.clear();
    QPainter paint_bitmap(&b);
    paint_bitmap.setBrush( Qt::color1 );
    paint_bitmap.setPen( Qt::color1 );
    paint_bitmap.drawRoundRect(0, 0, width()-1, height()-1, 40, 40);
    setMask(b);
}
