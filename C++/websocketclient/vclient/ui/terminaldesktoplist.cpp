#include "terminaldesktoplist.h"
#include "ui_terminaldesktoplist.h"

#include "sysbutton.h"
#include "../backend/csession.h"
#include "../common/log.h"
#include "../common/common.h"
#include "../common/errorcode.h"
#include "../common/cthread.h"
#include "cmessagebox.h"
#include "ui_interact_backend.h"
#include "cloadingdialog.h"
#include "desktopsettingdialog.h"
#include "terminaldesktoplist.h"
#include "../common/cconfiginfo.h"
#include "../imageconf.h"


#include <QtGui>
#include <QSizeGrip>
#include <iostream>




TerminaldesktopList::TerminaldesktopList(const std::vector<APP_LIST>& appList,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TerminaldesktopList),
    m_checkAll(false),
    m_oneCheck(false),
    m_applist(appList),
    m_loadingDlg(NULL),
    m_pQsizeGrip(NULL)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint |Qt::WindowSystemMenuHint| Qt::WindowMinimizeButtonHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    m_pixmapBackground.load(BACKGROUND);
    //ui->label_corner->setPixmap(QPixmap(CORNER_TOP_LEFT_IMG));
    ui->label_corner->setPixmap(QPixmap(vclient_image.corner_left_top.data()));
    ui->label_title->setText(tr("Terminal Control"));
    sysClose = new SysButton("icon_close.png", tr("Close"), this);
//    sysMinsize = new SysButton("icon_minimize.png", tr("Minimize"), this);


    connect(sysClose, SIGNAL(clicked()), this, SLOT(close()));

    ui->horizontalLayout_top->addWidget(sysClose);
//    ui->horizontalLayout_top->addWidget(sysMinsize);


    QPalette scrocontentPal = ui->scrollAreaWidgetContents->palette();
    scrocontentPal.setBrush(QPalette::Window, QBrush(QColor(0xfa, 0xfa, 0xfa)));//0xe9, 0xe9, 0xe9 scrocontentPal.setBrush(QPalette::Window, QBrush(QPixmap(SCROLLBACKGROUND)));
    ui->scrollAreaWidgetContents->setPalette(scrocontentPal);

    m_scrollAreaLayout = new QVBoxLayout(ui->scrollAreaWidgetContents);
    m_scrollAreaLayout->setContentsMargins(3, 0, 6, 0);//(6, 0, 6, 0)
    m_scrollAreaLayout->setSpacing(1);//default is 6

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 145, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_scrollAreaLayout->addItem(verticalSpacer);




    m_pQsizeGrip = new QSizeGrip(this);
    m_pQsizeGrip->setStyleSheet("image: url(:image/resource/image/icon_enlarge.png);");
    m_pQsizeGrip->resize(m_pQsizeGrip->sizeHint());
    m_pQsizeGrip->move(rect().bottomRight().x()-m_pQsizeGrip->rect().bottomRight().x() -15, \
                    rect().bottomRight().y()-m_pQsizeGrip->rect().bottomRight().y() -15);

    QString buttonStyle = "QPushButton{"
            "color: #515151;"
            "font: bold 11pt;"
            "border-radius: 0px;"
            "selection-color:white;}"
            "QPushButton:hover{"
            "color: #101010;"
            "font: bold ;}"
            "QPushButton:pressed{"
            "color: #000000;}";

    ui->pushBtn_poweroff->setStyleSheet(buttonStyle);
    ui->pushBtn_poweroff->setFocusPolicy(Qt::NoFocus);
    ui->pushBtn_restart->setStyleSheet(buttonStyle);
    ui->pushBtn_restart->setFocusPolicy(Qt::NoFocus);
    ui->pushBtn_msg->setStyleSheet(buttonStyle);
    ui->pushBtn_msg->setFocusPolicy(Qt::NoFocus);

    m_session = CSession::GetInstance();
    if(m_session != NULL)
    {
        strcpy(m_terminalDesktop.loginTicket, m_session->getNT_ACCOUNT_INFO().logonTicket);
    }
    connect(this, SIGNAL(on_signal_terminalDesktopCtl_finished(int,int)), this, SLOT(on_slot_terminalDesktopCtl_finised(int,int)));
    connect(ui->pushBtn_poweroff, SIGNAL(clicked()), this, SLOT(on_pushBtn_poweroff()));
    connect(ui->pushBtn_restart, SIGNAL(clicked()), this, SLOT(on_pushBtn_restart()));
    connect(ui->pushBtn_msg, SIGNAL(clicked()), this, SLOT(on_pushBtn_msg()));
    connect(ui->checkall, SIGNAL(stateChanged(int)), SLOT(on_slot_checkAll(int)));
    _createTableWidgetItem();
}


void TerminaldesktopList::_createTableWidgetItem()
{
    setLoadingDlg(true);
    for(std::vector<APP_LIST>::size_type i = 0; i < m_applist.size(); i++)
    {
        ITEM_TERMINALDATA data;
        CTerminalItem *item = getTerminalItem(m_applist.at(i).uuid);
        if(item == NULL)
        {
            item = new CTerminalItem(ui->scrollAreaWidgetContents);
            connect(item, SIGNAL(on_stateChanged_signal(string,int)), this, SLOT(on_stateChanged_checkTerminal(string,int)));
            data.appData = 0;
            memset(&data.ui_status, 0, sizeof(data.ui_status));
            data.appData = &m_applist[i];
            data.uuid = data.appData->uuid;
            item->setTerminalName(QString::fromUtf8(data.appData->TerminalName),data.appData->TerminalIp);
            item->setData(data);
            addTerminalItem(item);
        }
        else
        {
            data.appData = &m_applist[i];
            item->getTerminalData().appData = &m_applist[i];
            item->setTerminalName(QString::fromUtf8(data.appData->TerminalName),data.appData->TerminalIp);
            if(item->getTerminalData().ui_status.isRestart || item->getTerminalData().ui_status.isPoweroff)
                continue;
        }

    }
    setLoadingDlg(false);
//    _adjustItems();
}

CTerminalItem* TerminaldesktopList::getCheckTerminalItem(string uuid, int &vectorNum)
{
    CTerminalItem* item = NULL;
    for( int loop = 0; loop < m_stateCheckedItemVector.size(); loop++)
    {
        if( 0 == uuid.compare(m_stateCheckedItemVector[loop]->getTerminalData().uuid))
        {
            item = m_stateCheckedItemVector[loop];
            vectorNum = loop;
            break;
        }
    }
    return item;
}

void TerminaldesktopList::on_stateChanged_checkTerminal(string uuid, int state)
{
        int vectorNum;
        if( Qt::Checked == state)
        {
            CTerminalItem* item = getTerminalItem(uuid);
            if(item != NULL)
            {
                m_stateCheckedItemVector.append(item);
            }
        }
        else if( Qt::Unchecked == state)
        {
            CTerminalItem* item = getCheckTerminalItem(uuid, vectorNum);
            if(item != NULL)
            {
                m_stateCheckedItemVector.remove(vectorNum);
            }
        }
}

void TerminaldesktopList::on_slot_checkAll(int state)
{
    if(Qt::Checked == state)
    {
//        m_stateCheckedItemVector.clear();
        for(int loop = 0; loop < m_terminalItemVector.size(); loop++)
        {
//            m_stateCheckedItemVector.append(m_terminalItemVector[loop]);
            m_terminalItemVector[loop]->setChecked(true);
        }
    }
    else if( Qt::Unchecked == state)
    {
        for(int loop = 0; loop < m_terminalItemVector.size(); loop++)
        {
            m_terminalItemVector[loop]->setChecked(false);
        }
        m_stateCheckedItemVector.clear();
    }
}

CTerminalItem* TerminaldesktopList::getTerminalItem(string uuid)
{
    int num;
    return getTerminalItem(uuid, num);
}

CTerminalItem* TerminaldesktopList::getTerminalItem(string uuid, int &vectorNum)
{
    CTerminalItem* item = NULL;
    for( int loop = 0; loop < m_terminalItemVector.size(); loop++)
    {
        if( 0 == uuid.compare(m_terminalItemVector[loop]->getTerminalData().uuid))
        {
            item = m_terminalItemVector[loop];
            vectorNum = loop;
            break;
        }
    }
    return item;
}

void TerminaldesktopList::addTerminalItem(CTerminalItem *item)
{
    m_scrollAreaLayout->insertWidget(m_terminalItemVector.size(), item);
    m_terminalItemVector.append(item);
}
void TerminaldesktopList::_adjustItems()
{

}

void TerminaldesktopList::on_pushBtn_poweroff()
{
    m_terminalDesktop.uuid.clear();
    m_terminalDesktop.msg.clear();
    for(int loop =0; loop < m_stateCheckedItemVector.size(); loop++)
    {
        m_terminalDesktop.uuid.push_back(m_stateCheckedItemVector[loop]->getTerminalData().uuid);
    }
    if(m_terminalDesktop.uuid.size() == 0)
    {
        CMessageBox::TipBox(tr("you don't check one terminal desktop!"), this);
        return;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("s","new faild! NULL == pCall_param");
        return ;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = TERMINALDESKTOPLISTDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    setLoadingDlg(true);
    m_session->powerOffTerminal(param, m_terminalDesktop);
}
void TerminaldesktopList::on_pushBtn_restart()
{

    m_terminalDesktop.uuid.clear();
    m_terminalDesktop.msg.clear();
    for(int loop =0; loop < m_stateCheckedItemVector.size(); loop++)
    {
       m_terminalDesktop.uuid.push_back(m_stateCheckedItemVector[loop]->getTerminalData().uuid);
    }
    if(m_terminalDesktop.uuid.size() == 0)
    {
        CMessageBox::TipBox(tr("you don't check one terminal desktop!"), this);
        return;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("s","new faild! NULL == pCall_param");
        return ;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = TERMINALDESKTOPLISTDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    setLoadingDlg(true);
    m_session->restartTerminal(param, m_terminalDesktop);
}
void TerminaldesktopList::on_pushBtn_msg()
{
    m_terminalDesktop.uuid.clear();
    m_terminalDesktop.msg.clear();
    if(CMessageBox::messageBox(this) == ACCEPTED )
    {
        m_terminalDesktop.msg = CMessageBox::m_message;
        cout << m_terminalDesktop.msg.c_str() << endl;
    }
    else
    {
        return;
    }

    for(int loop =0; loop < m_stateCheckedItemVector.size(); loop++)
    {
        m_terminalDesktop.uuid.push_back(m_stateCheckedItemVector[loop]->getTerminalData().uuid);
    }
    if(m_terminalDesktop.uuid.size() == 0)
    {
        CMessageBox::TipBox(tr("you don't check one terminal desktop!"), this);
        return;
    }
    taskUUID taskUuid = TASK_UUID_NULL;
    CALLBACK_PARAM_UI *pCall_param = new CALLBACK_PARAM_UI;
    if(NULL == pCall_param)
    {
        LOG_ERR("s","new faild! NULL == pCall_param");
        return ;
    }
    pCall_param->pUi = this;
    pCall_param->uiType = TERMINALDESKTOPLISTDLG;
    memset(pCall_param->appUuid, 0, sizeof(pCall_param->appUuid));
    PARAM_SESSION_IN param;
    param.callbackFun = uiCallBackFunc;
    param.callback_param = pCall_param;
    param.isBlock = UNBLOCK;
    param.taskUuid = taskUuid;
    setLoadingDlg(true);
    m_session->messageTerminal(param, m_terminalDesktop);
}



void TerminaldesktopList::paintEvent(QPaintEvent *)
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
//        paint.drawRoundRect(QRectF(5+i, i, width()-11, height()-6), 2, 2);
//    }
//    for(int i=3; i>=2; i--)
//    {
//        color.setAlpha(70 - (i-1)*12);
//        pen.setColor(color);
//        paint.setPen(pen);
//        paint.drawRoundRect(QRectF(5+i, i, width()-11, height()-6), 2, 2);
//    }
//    for(int i=1; i>=0; i--)
//    {
//        color.setAlpha(100 - (i-1)*20);
//        paint.setPen(color);
//        paint.drawRoundRect(QRectF(5+i, i, width()-11, height()-6), 2, 2);
//    }
    paint.setBrush(QBrush(m_pixmapBackground));
    paint.setPen(QPen(QColor(0xe9, 0xe9, 0xe9)));
    paint.drawRoundRect(QRectF(0,0,width(),height()), 0,0);
//    paint.drawRoundRect(QRectF(5, 0, width()-11, height()-6), 2, 2);

    paint.setPen(QPen(QColor(0xcc, 0xcc, 0xcc)));
    QRect scrollcontent = ui->scrollWidget->rect();
    QRect scroll = ui->scrollAreaWidgetContents->rect();
    QPoint pos =ui->scrollWidget->pos();
    qDebug()<< pos.x() << pos.y() << endl;
    paint.drawLine(pos.x()+14,pos.y(),pos.x()+10+scroll.width(),pos.y());
    paint.drawLine(pos.x()+14,pos.y()+scrollcontent.height(),pos.x()+10+scroll.width(),pos.y()+scrollcontent.height());

    if(NULL != m_pQsizeGrip)
        m_pQsizeGrip->move(rect().bottomRight().x()-m_pQsizeGrip->rect().bottomRight().x() -7, \
                        rect().bottomRight().y()-m_pQsizeGrip->rect().bottomRight().y() -7);

}

void TerminaldesktopList::mouseMoveEvent(QMouseEvent *_me)
{
    if( (_me->buttons() == Qt::LeftButton) && m_isMove)
    {
        move( _me->globalPos() - m_pressPoint );
    }
    return QDialog::mouseMoveEvent(_me);
}
void TerminaldesktopList::mousePressEvent(QMouseEvent *_me)
{
        m_isMove = true;
        m_pressPoint = _me->pos();
    return QDialog::mousePressEvent(_me);
}
void TerminaldesktopList::mouseReleaseEvent(QMouseEvent *_me)
{
    m_isMove = false;
    return QDialog::mouseReleaseEvent(_me);
}

void TerminaldesktopList::setLoadingDlg(bool enable)
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

void TerminaldesktopList::processCallBackData(int errorCode, int dType, void *pRespondData)
{
        if( pRespondData != NULL)
        {
            delete pRespondData;
            pRespondData = NULL;
        }
        switch(dType)
        {
        case TYPE_POWEROFF_TERMINAL:
        {
            emit on_signal_terminalDesktopCtl_finished(errorCode, dType);
            break;
        }
        case TYPE_RESTART_TERMINAL:
        {
            emit on_signal_terminalDesktopCtl_finished(errorCode, dType);
            break;
        }
        case TYPE_MSG_TERMINAL:
        {
            emit on_signal_terminalDesktopCtl_finished(errorCode, dType);
            break;
        }
    }
}
void TerminaldesktopList::on_slot_terminalDesktopCtl_finised(int errorCode, int dType)
{
    setLoadingDlg(false);
    if(errorCode != 0)
    {
        processErrorCode(errorCode, dType);
    }
    else
    {
        CMessageBox::TipBox(tr("You have operate terminal desktop success!"));
        this->close();
    }
}

void TerminaldesktopList::processErrorCode(int errorCode, int dType)
{
    switch(dType)
    {
    case TYPE_POWEROFF_TERMINAL:
    {
        CMessageBox::CriticalBox(tr(" poweroff failed!"));
        break;
    }
    case TYPE_RESTART_TERMINAL:
    {
        CMessageBox::CriticalBox(tr("restart failed!"));
        break;
    }
    case TYPE_MSG_TERMINAL:
    {
        CMessageBox::CriticalBox(tr("send message failed!"));
        break;
    }
    default:
        showUiErrorCodeTip(errorCode);
        break;
    }
}

TerminaldesktopList::~TerminaldesktopList()
{
    if(NULL != m_loadingDlg)
    {
        m_loadingDlg->hide();
        delete m_loadingDlg;
        m_loadingDlg = NULL;
    }
    delete ui;
}
