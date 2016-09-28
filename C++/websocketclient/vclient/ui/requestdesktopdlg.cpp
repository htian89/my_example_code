#include "requestdesktopdlg.h"
#include "ui_requestdesktopdlg.h"
#include "ui_interact_backend.h"
#include "../common/log.h"
#include <errorcode.h>
#include <cmessagebox.h>
#include <config.h>
#include <stdio.h>
#include <QMouseEvent>
#include <QDialogButtonBox>
#include <QPushButton>
#include <desktoplistdialog.h>
#include "../imageconf.h"

RequestDesktopDlg::RequestDesktopDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RequestDesktopDlg),
    m_isMove(false),
    m_loadingDlg(NULL),
    z_psession(NULL)
{
    ui->setupUi(this);
    setWindowFlags( Qt::FramelessWindowHint| Qt::Dialog);
    backgroundPixmap.load(IMAGE_PATH_REQUESTDESKTOP_BACKGROUND);
    setAttribute(Qt::WA_TranslucentBackground);
    //ui->label->setPixmap(QPixmap(CORNER_TOP_LEFT_IMG));
    ui->label->setPixmap(QPixmap(vclient_image.corner_left_top.data()));
    QDialogButtonBox *requestButtonBox = new QDialogButtonBox();
    QPushButton *pushBtn_ok = new QPushButton(tr("OK"));
    QPushButton *pushBtn_cancel = new QPushButton(tr("Cancel"));
    pushBtn_ok->setFixedSize(70, 25);
    pushBtn_cancel->setFixedSize(70, 25);
    requestButtonBox->addButton(pushBtn_ok,QDialogButtonBox::AcceptRole);
    requestButtonBox->addButton(pushBtn_cancel,QDialogButtonBox::RejectRole);
    pushBtn_ok->setStyleSheet(STYLE_SHEET_PUSHBTN);
    pushBtn_cancel->setStyleSheet(STYLE_SHEET_PUSHBTN);

    ui->horizontalLayout_buttonBox->addWidget(requestButtonBox);
    QString strLineEdit ="QLineEdit{"
                "height:25px;"
                "width:260px;"
                "background-color: white;"
                "border-style:groove;"
                "border:1px groove #999999;"
                "padding: 0 8px;"
                "selection-background-color: darkgray;}";

    QString strDoubleSpinBox = "QDoubleSpinBox{"
            "border-radius: 2px;"
            "border-style:groove;"
            "border:1px groove #999999;"
            "selection-background-color: darkgray;}";

    ui->comboBox_cpu->setStyleSheet(STYLE_SHEET_COMBO_BOX_OTHER);
    ui->comboBox_os->setStyleSheet(STYLE_SHEET_COMBO_BOX_OTHER);
    ui->lineEdit_discription->setStyleSheet(strLineEdit);
    ui->doubleSpinBox_disk->setStyleSheet(strDoubleSpinBox);
    ui->doubleSpinBox_memory->setStyleSheet(strDoubleSpinBox);

    z_psession = CSession::GetInstance();
    if(z_psession != NULL)
    {
        strcpy(z_requestDesktop.logonTicket, z_psession->getNT_ACCOUNT_INFO().logonTicket);
    }
    ui->doubleSpinBox_memory->setValue(2);
    ui->doubleSpinBox_disk->setValue(20);
    ui->comboBox_cpu->currentIndex();
    ui->comboBox_os->currentIndex();

    connect(pushBtn_ok, SIGNAL(clicked()), this, SLOT(on_pushBtn_ok_clicked()));
    connect(pushBtn_cancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(this, SIGNAL(on_signal_requestDesktop_finished(int,int)), this, SLOT(on_requestDesktop_finished(int,int)));

}

void RequestDesktopDlg::setLoadingDlg(bool enable)
{
    if(m_loadingDlg==NULL)
        m_loadingDlg = new CLoadingDialog(tr("Operate the request ..."), this);
    else
        m_loadingDlg->setText(tr("Operate the request ..."));
    QRect rect = geometry();
    m_loadingDlg->setPos(0, 5, rect.width(), rect.height()-5);
    m_loadingDlg->setMovieStop(enable^1);
    m_loadingDlg->setVisible(enable);
}

void RequestDesktopDlg::on_pushBtn_ok_clicked()
{
    z_requestDesktop.Cpu = ui->comboBox_cpu->currentText().toUtf8().data();
    char strMemory[MIN_LEN];
    sprintf(strMemory, "%gG",ui->doubleSpinBox_memory->value());
    z_requestDesktop.Memory = strMemory;
    z_requestDesktop.Os = ui->comboBox_os->currentText().toUtf8().data();
    char strDisk[MIN_LEN];
    sprintf(strDisk, "%gG", ui->doubleSpinBox_disk->value());
    z_requestDesktop.Disk = strDisk;
    z_requestDesktop.desktopDescrip = ui->lineEdit_discription->text().left(200).toUtf8().data();

    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("s","new faild! NULL == pCall_param");
        return ;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = REQUESTDESKTOP;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;

    setLoadingDlg(true);

    z_psession->requestDesktop(param,z_requestDesktop);
}

void RequestDesktopDlg::processCallBackData(int errorCode, int dType, void *pRespondData)
{
    if( dType == TYPE_REQUEST_DESKTOP )
    {
        emit on_signal_requestDesktop_finished(errorCode, dType);
        if(pRespondData!=NULL)
            delete pRespondData;
    }

}
void RequestDesktopDlg::processErrorCode(int errorCode, int opType)
{
    LOG_ERR("errcode:%d\t\t opType:%d", errorCode, opType);
    switch(errorCode)
    {
    case ERROR_FAIL:
    {
        CMessageBox::CriticalBox(tr("RequestDesktop failed!"),this);
        break;
    }
    case ERROR_PARAMS:
    {
        CMessageBox::CriticalBox(tr("Request params error!"),this);
        break;
    }
    default:
        showUiErrorCodeTip(errorCode);
        break;
    }
}

void RequestDesktopDlg::on_requestDesktop_finished(int errorCode, int dType)
{
    if(errorCode != 0)
    {
        setLoadingDlg(false);
        processErrorCode(errorCode, dType);
    }
    else
    {
        setLoadingDlg(false);
        CMessageBox::TipBox(tr("You have request desktop success!"),this);
        close();
    }
}
void RequestDesktopDlg::mouseMoveEvent(QMouseEvent *_qm)
{
    if(_qm->buttons() == Qt::LeftButton && m_isMove)
        move(_qm->globalPos() - m_pressPoint);
}

void RequestDesktopDlg::mousePressEvent(QMouseEvent *_qm)
{
    m_isMove = true;
    m_pressPoint = _qm->pos();
}

void RequestDesktopDlg::mouseReleaseEvent(QMouseEvent *)
{
    m_isMove = false;
}

void RequestDesktopDlg::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setBrush(Qt::NoBrush);
    QPen pen;
    QColor color(0, 0, 0, 50);
//    for(int i=5; i>=4; i--)
//    {
//        color.setAlpha(50 - (i-1)*10);
//        pen.setColor(color);
//        paint.setPen(pen);
//        paint.drawRoundRect(QRectF(1+i, 1+i, width()-7, height()-7), 3, 3);
//    }
//    for(int i=3; i>=2; i--)
//    {
//        color.setAlpha(70 - (i-1)*12);
//        pen.setColor(color);
//        paint.setPen(pen);
//        paint.drawRoundRect(QRectF(1+i, 1+i, width()-7, height()-7), 3, 3);
//    }
//    for(int i=1; i>=0; i--)
//    {
//        color.setAlpha(100 - (i-1)*20);
//        paint.setPen(color);
//        paint.drawRoundRect(QRectF(1+i, 1+i, width()-7, height()-7), 3, 3);
//    }
    paint.setBrush(QBrush(backgroundPixmap));
    paint.setPen(Qt::transparent);
    paint.drawRoundRect(QRectF(0,0,width(),height()),0,0);
//    paint.drawRoundRect(QRectF(0, 0, width()-6, height()-6), 3, 3);
}
RequestDesktopDlg::~RequestDesktopDlg()
{
    if(NULL != m_loadingDlg)
    {
        m_loadingDlg->hide();
        delete m_loadingDlg;
        m_loadingDlg = NULL;
    }

    delete ui;
}
