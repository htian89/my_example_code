#include "desktopsettingdialog.h"
#include "ui_desktopsettingdialog.h"
#include "../config.h"
#include "../backend/csession.h"
#include "cdesktoplistitem.h"
#include "common/log.h"
#include "../common/cconfiginfo.h"
#include "cloadingdialog.h"
#include "ui_interact_backend.h"
#include "cmessagebox.h"
#include "titlewidget.h"
#include <QMouseEvent>
#include <QDebug>
#include "../imageconf.h"

extern CConfigInfo* g_pConfigInfo; //defined in main.cpp

DesktopSettingDialog::DesktopSettingDialog(const DESKTOP_SETTING_PARAM &param, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DesktopSettingDialog),
    m_isMove(false),
    m_param(param)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint|Qt::WindowMinimizeButtonHint | Qt::Dialog);
    //setWindowIcon(QIcon(WINDOWS_ICON));
    setWindowIcon(QIcon(WINDOWS_IMG.data()));
    QPalette pal = palette();
    pal.setBrush(QPalette::Window, QBrush(QPixmap(IMAGE_PATH_DESKTOP_SETTING_BACKGROUND)));
    setPalette(pal);
    //setAttribute(Qt::WA_DeleteOnClose);       
    m_titleWidget = new TitleWidget(ONLYCLOSE, "", this);
    m_titleWidget->setImage("icon_close.png", CLOSE);
    m_titleWidget->setLayout_title(0, 0, 3, 0);

    //ui->label_8->setPixmap(QPixmap(CORNER_TOP_LEFT_IMG));
    ui->label_8->setPixmap(QPixmap(vclient_image.corner_left_top.data()));
    ui->pushBtn_ok->setStyleSheet(STYLE_SHEET_PUSHBTN);
    ui->comboBox_pl->setStyleSheet(STYLE_SHEET_COMBO_BOX);
    ui->comboBox_ps->setStyleSheet(STYLE_SHEET_COMBO_BOX);
    ui->comboBox_sl->setStyleSheet(STYLE_SHEET_COMBO_BOX);
    ui->comboBox_ss->setStyleSheet(STYLE_SHEET_COMBO_BOX);

//#ifdef VERSION_VSAP
//    ui->comboBox_ptotocal->setItemText(1, tr("VSAP"));
//#endif

    m_pSession = CSession::GetInstance();
    if(m_pSession == NULL)
        return;

    if(m_param.item==NULL)
        LOG_ERR("%s", "The parent is null so can not initial the desktop setting dialog");
    else
    {
        ITEM_DATA& data = m_param.item->getData();

        ui->comboBox_pl->setCurrentIndex(data.ui_status.stPort.parallelLocal);
        ui->comboBox_ps->setCurrentIndex(data.ui_status.stPort.parallelRemote);
        ui->comboBox_sl->setCurrentIndex(data.ui_status.stPort.serialLocal);
        ui->comboBox_ss->setCurrentIndex(data.ui_status.stPort.serialRemote);

        /***
         *whether personal disk can be checked of not
         ****/
//        if(param.hasVirtualDisk && data.appData->desktopType == DESKTOPPOOL && data.appData->sourceType==0)
//        {
//            ui->checkBox_virtualdisk->setEnabled(true);
//            if(data.ui_status.hasVDisk)
//                ui->checkBox_virtualdisk->setChecked(true);
//        }
//        else
//        {
//            ui->checkBox_virtualdisk->setEnabled(false);
//            if(!param.hasVirtualDisk)
//                ui->label_disktip->setText(tr("(No virtual disk!)"));
//        }

//        /****
//         * Whether serial port combox can be selected or not
//         ****/
//        if(data.appData->resParams.serialPort > 0)
//        {
//            ui->comboBox_sl->setCurrentIndex(data.ui_status.stPort.serialLocal);
//            ui->comboBox_ss->setCurrentIndex(data.ui_status.stPort.serialRemote);
//        }
//        else
//            ui->groupBox_serial->setEnabled(false);

//        /***
//         * Whether parallel port combox can be selected or not
//         ****/
//        if(data.appData->resParams.parallelPort > 0)
//        {
//            ui->comboBox_pl->setCurrentIndex(data.ui_status.stPort.parallelLocal);
//            ui->comboBox_ps->setCurrentIndex(data.ui_status.stPort.parallelRemote);
//        }
//        else
//            ui->groupBox_parallel->setEnabled(false);

//        /**
//         * Set the default desktop status.
//         *Only the uuid , username , server address are all the same with the
//         *default desktop/app information read from file can be set the ui->checkBox_setDefault
//         *checked.
//         **/
//        if(g_pConfigInfo!=NULL)
//        {
//            SETTING_DEFAULTAPP default_desktop ;
//            g_pConfigInfo->getSetting_defaultDesktop(default_desktop);
//            if(data.ui_status.support_protocal==BOTH_SUPPORT)
//                ui->comboBox_ptotocal->setCurrentIndex(default_desktop.connectProtocal);
//            else
//            {
//                ui->comboBox_ptotocal->setEnabled(false);
//                if(data.ui_status.support_protocal==ONLY_FAP)
//                    ui->comboBox_ptotocal->setCurrentIndex(1);
//            }
////            if(QString(default_desktop.uuid) == data.uuid &&
////                    QString(m_pSession->getUSER_INFO().username) == QString(default_desktop.userName) &&
////                    QString(m_pSession->getNetwork().presentServer) == QString(default_desktop.serverIp))
//            if(QString(default_desktop.uuid) == data.uuid &&
//                    QString(m_pSession->getUSER_INFO().username) == QString(default_desktop.userName) &&
//                    QString(m_pSession->getNetwork().stPresentServer.serverAddress) == QString(default_desktop.serverIp))
//            {
//                if(default_desktop.isAutoConnect)
//                    ui->checkBox_setDefault->setChecked(true);
//            }
//        }
        if(NULL != data.appData && strlen(data.appData->uuid)>0)
        {
            CALLBACK_PARAM_UI* pstCall_param = new CALLBACK_PARAM_UI;
            pstCall_param->pUi = this;
            pstCall_param->uiType = DESKTOP_SETTING_DLG;
            pstCall_param->errorCode = 0;
            PARAM_SESSION_IN param;
            param.callback_param = pstCall_param;
            param.callbackFun = &uiCallBackFunc;
            param.isBlock = UNBLOCK;
            param.taskUuid = TASK_UUID_NULL;
            m_pSession->getResParam(param, data.appData->uuid, int(data.appData->desktopType));
            connect(this, SIGNAL(getResParametersFinished(int,GET_RESOURCE_PARAMETER)),\
                    this, SLOT(on_getResParametersFinished(int,GET_RESOURCE_PARAMETER)));
            m_dlg_loading = new CLoadingDialog(tr("get resouce parameters"), this);

            m_dlg_loading->setPos(0, 5, width(), height());
            m_dlg_loading->setMovieStop(false);
            m_dlg_loading->setVisible(true);
            ui->widget->setVisible(false);
        }
    }    
}

DesktopSettingDialog::~DesktopSettingDialog()
{
    delete ui;
    if(NULL != m_dlg_loading)
    {
        delete m_dlg_loading;
    }
}

void DesktopSettingDialog::on_pushBtn_ok_clicked()
{
//    if(m_pSession==NULL)
//        return;
    if(NULL != m_param.item)
    {
        UI_STATUS &ui_status = m_param.item->getData().ui_status;
        //ui_status.hasVDisk = ui->checkBox_virtualdisk->isChecked();
        ui_status.stPort.serialLocal = ui->comboBox_sl->currentIndex();
        ui_status.stPort.serialRemote = ui->comboBox_ss->currentIndex();
        ui_status.stPort.parallelLocal = ui->comboBox_pl->currentIndex();
        ui_status.stPort.parallelRemote = ui->comboBox_ps->currentIndex();
    }
    else
    {
        LOG_ERR("%s","NULL==m_param.item");
    }

//    if(g_pConfigInfo == NULL)
//        close();
//    SETTING_DEFAULTAPP default_desktop;
//    g_pConfigInfo->getSetting_defaultDesktop(default_desktop) ;
//    ITEM_DATA item_data = m_param.item->getData();
//    if(QString(default_desktop.uuid) != item_data.uuid)
//    {
//        if(ui->checkBox_setDefault->isChecked())
//        {
//            strcpy(default_desktop.uuid, item_data.uuid.toLocal8Bit().data());
//            strcpy(default_desktop.appName,  item_data.appData->name);
//            strcpy(default_desktop.serverIp, m_pSession->getNetwork().stPresentServer.serverAddress);//strcpy(default_desktop.serverIp, m_pSession->getNetwork().presentServer);
//            strcpy(default_desktop.userName, m_pSession->getUSER_INFO().username);
//            default_desktop.connectProtocal = ui->comboBox_ptotocal->currentIndex();
//            default_desktop.isAutoConnect = ui->checkBox_setDefault->isChecked();
//            //default_desktop.isLoadvDisk = ui->checkBox_virtualdisk->isChecked();
//            default_desktop.isMapUsb = 0;
//            g_pConfigInfo->setSetting_defaultDesktop(default_desktop);
//        }
//    }
//    else
//    {
//        //default_desktop.isLoadvDisk = ui->checkBox_virtualdisk->isChecked();
//        strcpy(default_desktop.serverIp, m_pSession->getNetwork().stPresentServer.serverAddress);//strcpy(default_desktop.serverIp, m_pSession->getNetwork().presentServer);
//        strcpy(default_desktop.userName, m_pSession->getUSER_INFO().username);
//        default_desktop.isAutoConnect = ui->checkBox_setDefault->isChecked();
//        default_desktop.connectProtocal = ui->comboBox_ptotocal->currentIndex();
//        g_pConfigInfo->setSetting_defaultDesktop(default_desktop);
//    }

    close();
}

void DesktopSettingDialog::on_getResParametersFinished(int errorCode, GET_RESOURCE_PARAMETER stResParam)
{
    if(errorCode < 0)
    {
        CMessageBox::CriticalBox(tr("Get Resourece(config) Parameter failed."));
        //ui->groupBox_serial->setEnabled(false);
        //ui->groupBox_parallel->setEnabled(false);
        ui->comboBox_pl->setEnabled(false);
        ui->comboBox_ps->setEnabled(false);
        ui->comboBox_sl->setEnabled(false);
        ui->comboBox_ss->setEnabled(false);
        LOG_ERR("RESPARAMTER failed. return value:%d", errorCode);
    }
    else
    {
        if(NULL != m_param.item)
        {
            APP_LIST * pAppList = m_param.item->getData().appData;
            ITEM_DATA data = m_param.item->getData();
            if(NULL != pAppList)
            {
                if(0 != strcmp(stResParam.deskUuid, pAppList->uuid))
                    LOG_ERR("ERROR:stResParam.deskUuid!= pAppList->uuid, PLEASE CHECK CAREFULLY!!!!!!!!%s\t\t%s",stResParam.deskUuid, pAppList->uuid);
                pAppList->resParams = stResParam.stResPara;

                /****
                 * Whether serial port combox can be selected or not
                 ****/
                if(data.appData->resParams.serialPort > 0)
                {
                    ui->comboBox_sl->setCurrentIndex(data.ui_status.stPort.serialLocal);
                    ui->comboBox_ss->setCurrentIndex(data.ui_status.stPort.serialRemote);
                }
                else
                {
                    ui->comboBox_sl->setEnabled(false);
                    ui->comboBox_ss->setEnabled(false);
                    //ui->groupBox_serial->setEnabled(false);
                }

                /***
                 * Whether parallel port combox can be selected or not
                 ****/
                if(data.appData->resParams.parallelPort > 0)
                {
                    ui->comboBox_pl->setCurrentIndex(data.ui_status.stPort.parallelLocal);
                    ui->comboBox_ps->setCurrentIndex(data.ui_status.stPort.parallelRemote);
                }
                else
                {
                    ui->comboBox_pl->setEnabled(false);
                    ui->comboBox_ps->setEnabled(false);
                    //ui->groupBox_parallel->setEnabled(false);
                }
            }
        }
    }
    disconnect(this, SIGNAL(getResParametersFinished(int,GET_RESOURCE_PARAMETER)),\
               this, SLOT(on_getResParametersFinished(int,GET_RESOURCE_PARAMETER)));
    if(NULL != m_dlg_loading)
    {
        m_dlg_loading->setMovieStop(true);
        m_dlg_loading->setVisible(false);
    }
    ui->widget->setVisible(true);
}

void DesktopSettingDialog::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    paint.setBrush(Qt::NoBrush);
    QPen pen;
    QColor color(0, 0, 0, 50);
//    for(int i=5; i>=3; i--)
//    {
//        color.setAlpha(20 - (i-1)*5);
//        pen.setColor(color);
//        paint.setPen(pen);
//        paint.drawRoundRect(QRectF(1+i, 1+i, width()-6, height()-6), 2, 2);
//    }
//    for(int i=2; i>=0; i--)
//    {
//        color.setAlpha(80 - (i-1)*5);
//        paint.setPen(color);
//        paint.drawRoundRect(QRectF(1+i, 1+i, width()-6, height()-6), 2, 2);
//    }
    paint.setBrush(QBrush(QPixmap(BACKGROUND_IMAGE)));
    paint.setPen(QPen(QColor(0xe9, 0xe9, 0xe9)));
//    paint.drawRoundRect(QRectF(0, 0, width()-5, height()-5), 2, 2);
     paint.drawRoundRect(QRectF(0, 0, width(), height()), 0, 0);

//    QPainter paint(this);
////    QPainterPath path;
////    path.setFillRule(Qt::WindingFill);
////    path.addRect(5, 0, this->width()-10, this->height()-5);

////    paint.setRenderHint(QPainter::Antialiasing, true);
////    paint.fillPath(path, QBrush(QPixmap(BACKGROUND_IMAGE)));

//    paint.setBrush(QBrush(QPixmap(BACKGROUND_IMAGE)));
//    QRectF rectfContent(0, 0, width()-5, height()-5);
//    paint.setPen(QPen(QColor(0xe9, 0xe9, 0xe9)));
//    paint.drawRoundRect(rectfContent, 2, 2);

//    QColor color(0, 0, 0, 50);
//    for(int i=0; i<2; i++)
//    {
//        color.setAlpha(80 - (i-1)*5);
//        paint.setPen(color);
//        //paint.drawLine(10-i, i, 10-i, height()-(11-i));
//        paint.drawLine(width()-(5-i), i+2, width()-(5-i), height()-(5-i));
//        paint.drawLine(i+2, height()-(5-i), width()-(6-i), height()-(5-i));
//    }
//    for(int i=2; i<5; i++)
//    {
//        color.setAlpha(20 - (i-1)*5);
//        paint.setPen(color);
//        //paint.drawLine(5-i, i, 5-i, height()-(6-i));
//        paint.drawLine(width()-(5-i), i+2, width()-(5-i), height()-(5-i));
//        paint.drawLine(i+2, height()-(5-i), width()-(6-i), height()-(5-i));
//    }
}

//void DesktopSettingDialog::mouseMoveEvent(QMouseEvent *_qm)
//{
//    if(_qm->buttons() == Qt::LeftButton && m_isMove)
//        move(_qm->globalPos() - m_pressPoint);
//}

//void DesktopSettingDialog::mousePressEvent(QMouseEvent *_qm)
//{
//    m_isMove = true;
//    m_pressPoint = _qm->pos();
//}

//void DesktopSettingDialog::mouseReleaseEvent(QMouseEvent *)
//{
//    m_isMove = false;
//}

void DesktopSettingDialog::closeEvent(QCloseEvent *)
{
    disconnect(this, SIGNAL(getResParametersFinished(int,GET_RESOURCE_PARAMETER)),\
            this, SLOT(on_getResParametersFinished(int,GET_RESOURCE_PARAMETER)));
    if(m_param.item!=NULL)
    {
        ITEM_DATA& data = m_param.item->getData();
        if( ui->comboBox_pl->currentIndex()!= data.ui_status.stPort.parallelLocal ||
                ui->comboBox_ps->currentIndex() != data.ui_status.stPort.parallelRemote ||
                ui->comboBox_sl->currentIndex()!= data.ui_status.stPort.serialLocal ||
                ui->comboBox_ss->currentIndex()!= data.ui_status.stPort.serialRemote)
        {//changed. ask user to  apply or not
            if(CMessageBox::SelectedBox(tr("Are you sure to apply the settings?")) == ACCEPTED)
            {
                data.ui_status.stPort.parallelLocal = ui->comboBox_pl->currentIndex();
                data.ui_status.stPort.parallelRemote = ui->comboBox_ps->currentIndex();
                data.ui_status.stPort.serialLocal = ui->comboBox_sl->currentIndex();
                data.ui_status.stPort.serialRemote = ui->comboBox_ss->currentIndex();
            }
        }
    }
}

void DesktopSettingDialog::processCallBackData(int errorCode, int dType, void *pRespondData)
{
    switch(dType)
    {
    case TYPE_GET_RESPARAMTER:
    {
        GET_RESOURCE_PARAMETER* pData = (GET_RESOURCE_PARAMETER*)pRespondData;
        GET_RESOURCE_PARAMETER st = *pData;
        emit getResParametersFinished(errorCode, st);
        delete pData;
        break;
    }
    }
}
