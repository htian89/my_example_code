#include "cmessagebox.h"
#include "titlewidget.h"
#include "ui_cmessagebox.h"
#include "../config.h"
#include "../common/common.h"
#include "sysbutton.h"
#include <QDesktopWidget>
#include "../common/log.h"
#include "../imageconf.h"
#include "ui/autologindialog.h"
#include "../ipc/ipcclient.h"
#include <QWidget>

#define PUSHBTN_WIDTH 66
#define PUSHBTN_HEIGHT 26

//extern AutoLoginDialog *g_autoLoginDlg;
string CMessageBox::m_message;
extern CMessageBox *g_cmessagebox;
extern IpcClient *g_ipcClient;
extern QTextCodec *codec;
extern char notes[MAX_LEN] ;

CMessageBox::CMessageBox(QWidget *parent) :
    QDialog(parent),
    m_pUi(new Ui::CMessageBox),
    m_isMove(false)
{
    m_pUi->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);
    //setWindowIcon(QIcon(WINDOWS_ICON));
    setWindowIcon(QIcon(WINDOWS_IMG.data()));
    //m_pUi->label_corner->setPixmap(QPixmap(CORNER_TOP_LEFT_IMG));
    m_pUi->label_corner->setPixmap(QPixmap(vclient_image.corner_left_top.data()));
    m_pUi->label_image2->hide();//in about dlg, it shold has two images
//    QPalette pal = palette();
//    pal.setBrush(QPalette::Window, QBrush(QPixmap(BACKGROUND_IMAGE)));
//    setPalette(pal);
    setDialogInCenter(this);

    QString styleSheet = "QWidget#widget_content{"
            "background-color:#e9e9e9;}";
    m_pUi->widget_content->setStyleSheet(styleSheet);
    setStyleSheet(STYLE_SHEET_PUSHBTN);

    connect(this, SIGNAL(on_signal_setSeatNumber()), this, SLOT(on_setSeatNumber()));
    connect(this, SIGNAL(on_signal_showSeatNumber()), this, SLOT(on_showSeatNumber()));
    connect(this, SIGNAL(on_signal_setSeatNumber_finished()), this, SLOT(on_finish_setSeatNumber()));
    connect(this, SIGNAL(on_signal_showNotes()), this, SLOT(on_showNotes()));

    //m_pUi->widget_content->setStyleSheet("background-color: 0;");//#e9e9e9//m_pUi->widget_content->setStyleSheet("background-color: rgb(234,234,234);");//
    if(parent == NULL)
        move((qApp->desktop()->availableGeometry().width()-width())/2,
             (qApp->desktop()->availableGeometry().height()- height())/2);
}

CMessageBox::~CMessageBox()
{
    delete m_pUi;
}

void CMessageBox::setImage(QPixmap pixmap)
{
    m_pUi->label_image->setPixmap(pixmap);
}

void CMessageBox::setImage2(QPixmap pixmap, bool bShow)
{
    m_pUi->label_image2->setPixmap(pixmap);
    if(bShow)
        m_pUi->label_image2->show();
    else
        m_pUi->label_image2->hide();
}

void CMessageBox::addButton(QPushButton *btn)
{
    m_pUi->horizontalLayout_button->insertWidget(1, btn);
}

void CMessageBox::setTitle(QString title)
{
    m_pUi->label_title->setText(title);
}

void CMessageBox::setContentText(QString text)
{
    m_pUi->label_text->setText(text);
}

void CMessageBox::paintEvent(QPaintEvent *)
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
    paint.setBrush(QBrush(QPixmap(BACKGROUND_IMAGE)));
    paint.setPen(Qt::transparent);
//    paint.drawRoundRect(QRectF(1, 0, width()-7, height()-6), 3, 3);
     paint.drawRoundRect(QRectF(0, 0, width(), height()), 0, 0);

//    QPainter paint(this);
//    QPainterPath path;
//    path.setFillRule(Qt::WindingFill);
//    path.addRect(5, 0, this->width()-10, this->height()-5);

//    paint.setRenderHint(QPainter::Antialiasing, true);
//    paint.fillPath(path, QBrush(QPixmap(BACKGROUND_IMAGE)));

//    QColor color(0, 0, 0, 50);
//    for(int i=0; i<2; i++)
//    {
//        QPainterPath path;
//        path.setFillRule(Qt::WindingFill);
//        color.setAlpha(80 - (i-1)*5);
//        paint.setPen(color);
//        //paint.drawLine(10-i, i, 10-i, height()-(11-i));
//        paint.drawLine(width()-(5-i), i+2, width()-(5-i), height()-(6-i));
//        paint.drawLine(7-i, height()-(5-i), width()-(6-i), height()-(5-i));
//    }
//    for(int i=2; i<5; i++)
//    {
//        QPainterPath path;
//        path.setFillRule(Qt::WindingFill);
//        color.setAlpha(20 - (i-1)*5);
//        paint.setPen(color);
//        //paint.drawLine(5-i, i, 5-i, height()-(6-i));
//        paint.drawLine(width()-(5-i), i+2, width()-(5-i), height()-(6-i));
//        paint.drawLine(7-i, height()-(5-i), width()-(6-i), height()-(5-i));
//    }
////    for(int i=5; i<10; i++)
////    {
////        QPainterPath path;
////        path.setFillRule(Qt::WindingFill);
////        color.setAlpha(20 - (i-1)*2);
////        paint.setPen(color);
////        paint.drawLine(5-i, i, 5-i, height()-(6-i));
////        paint.drawLine(width()-(5-i), i, width()-(5-i), height()-(6-i));
////        paint.drawLine(5-i, height()-(5-i), width()-(6-i), height()-(5-i));
////    }

}

void CMessageBox::mousePressEvent(QMouseEvent *_me)
{
    m_pressPoint = _me->pos();
    m_isMove = true;
}

void CMessageBox::mouseMoveEvent(QMouseEvent *_me)
{
    if((_me->buttons() == Qt::LeftButton) && m_isMove)
        move(_me->globalPos() - m_pressPoint);
}

void CMessageBox::mouseReleaseEvent(QMouseEvent *)
{
    m_isMove = false;
}

QString CMessageBox::btStyleSheet = "QPushButton{"
        "color: rgb(255, 255, 255);"
        "font: bold 14px;"
        "border-radius: 0px;"
        "padding: 3px 8px;"
        "background-color: #669900;}"
        "QPushButton:hover{"
        "background-color: #77b200;}"
        "QPushButton:pressed{"
        "background-color: #99cc00;}";

CMSGBOXVALUE CMessageBox::SelectedBox(const QString &text, QWidget *parent)
{
    CMessageBox *box = new CMessageBox(parent);
    QPixmap pixmap(SELECT_IAMGE);
    box->setWindowTitle(tr("Tip"));
    box->setTitle(tr("Tip"));
    box->setImage(pixmap.scaled(QSize(26, 26)));
    box->setContentText(text);
    QPushButton *btnOk = new QPushButton(tr("Ok"));
    QPushButton *btnNo = new QPushButton(tr("No"));
    btnOk->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
    btnNo->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
    //btnOk->setStyleSheet(btStyleSheet);
    //btnNo->setStyleSheet(btStyleSheet);
    connect(btnOk, SIGNAL(clicked()), box, SLOT(accept()));
    connect(btnNo, SIGNAL(clicked()), box, SLOT(reject()));
    btnOk->setFocusPolicy(Qt::NoFocus);
    btnNo->setFocusPolicy(Qt::NoFocus);
    box->addButton(btnNo);
    box->addButton(btnOk);
    box->resize(box->sizeHint());
    int result = box->exec();
    if(result == 0)
        return REJECTED;
    else
        return ACCEPTED;
}

void CMessageBox::on_text_change(QString msg)
{
    m_message.clear();
    m_message = msg.toUtf8().data();
}

void CMessageBox::on_text_change_seat(QString text)
{
    g_cmessagebox->setSeat = text;
    LOG_INFO("get seat number is %s", g_cmessagebox->setSeat.toStdString().c_str());
}

void CMessageBox::on_slot_setseatNumber()
{
    if (g_cmessagebox->setSeat.isEmpty())
    {
        CMessageBox::CriticalBox(tr("the seatnumber is empty"));
        return;
    }
    accept();
}

CMSGBOXVALUE CMessageBox::messageBox(QWidget *parent)
{
    CMessageBox *box = new CMessageBox(parent);
    QPixmap pixmap(SELECT_IAMGE);
    box->setWindowTitle(tr("Messaage"));
    box->setTitle(tr("Message"));
    box->setImage(pixmap.scaled(QSize(26, 26)));
    QLineEdit *lineEdit = new QLineEdit(box);
    lineEdit->setMaxLength(100);
    connect(lineEdit, SIGNAL(textChanged(QString)), box, SLOT(on_text_change(QString)));
    box->m_pUi->horizontalLayout_3->addWidget(lineEdit);
    box->m_pUi->label_text->hide();
    QPushButton *btnOk = new QPushButton(tr("Ok"));
    QPushButton *btnNo = new QPushButton(tr("No"));
    btnOk->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
    btnNo->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
    //btnOk->setStyleSheet(btStyleSheet);
    //btnNo->setStyleSheet(btStyleSheet);
    connect(btnOk, SIGNAL(clicked()), box, SLOT(accept()));
    connect(btnNo, SIGNAL(clicked()), box, SLOT(reject()));
    btnOk->setFocusPolicy(Qt::NoFocus);
    btnNo->setFocusPolicy(Qt::NoFocus);
    box->addButton(btnNo);
    box->addButton(btnOk);
    box->resize(box->sizeHint());
    int result = box->exec();

    if(result == 0)
        return REJECTED;
    else
    {
        return ACCEPTED;
    }
}

CMSGBOXVALUE CMessageBox::messageBox_setseatNumber(QWidget *parent)
{
    CMessageBox *box = new CMessageBox(parent);
    QPixmap pixmap(SELECT_IAMGE);
    box->setWindowTitle(tr("setSeatNumber"));
    box->setTitle(tr("setSeatNumber"));
    box->setImage(pixmap.scaled(QSize(26, 26)));
    QLineEdit *lineEdit = new QLineEdit(box);
    lineEdit->setMaxLength(100);
    //connect(lineEdit, SIGNAL(textChanged(QString)), box, SLOT(on_text_change_seat(QString)));
    connect(lineEdit, SIGNAL(textChanged(QString)), box, SLOT(on_text_change_seat(QString)));
    box->m_pUi->horizontalLayout_3->addWidget(lineEdit);
    if (!g_cmessagebox->setSeat.isEmpty())
        g_cmessagebox->setSeat.clear();
    box->m_pUi->label_text->hide();
    box->m_pUi->label_image->hide(); //hide ?
    QPushButton *btnOk = new QPushButton(tr("OK"));
    QPushButton *btnNo = new QPushButton(tr("NO"));
    btnOk->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
    btnNo->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
    //btnOk->setStyleSheet(btStyleSheet);
    //btnNo->setStyleSheet(btStyleSheet);
    //connect(btnOk, SIGNAL(clicked()), box, SLOT(accept()));
    connect(btnOk, SIGNAL(clicked()), box, SLOT(on_slot_setseatNumber()));
    connect(btnNo, SIGNAL(clicked()), box, SLOT(reject()));
    btnOk->setFocusPolicy(Qt::NoFocus);
    btnNo->setFocusPolicy(Qt::NoFocus);
    box->addButton(btnNo);
    box->addButton(btnOk);
    box->resize(box->sizeHint());
    int result = box->exec();

    if(result == 0)
        return REJECTED;
    else
    {
        return ACCEPTED;
    }
}

void CMessageBox::TipBox_seatNumber(const QString &text, QWidget *parent)
{
    QPixmap pixmap(SELECT_IAMGE);
    CMessageBox *box = new CMessageBox(parent);
    box->setWindowTitle(tr("Tip"));
    box->setTitle(tr("The SeatNumber"));
    box->setImage(pixmap.scaled(QSize(26, 26)));
    box->setContentText(text);
    QPushButton *btnOk = new QPushButton(tr("Ok"));
    btnOk->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
    //btnOk->setStyleSheet(btStyleSheet);
    connect(btnOk, SIGNAL(clicked()), box, SLOT(close()));
    box->m_pUi->label_image->hide(); //hide ?
    btnOk->setFocusPolicy(Qt::NoFocus);
    box->addButton(btnOk);
    box->resize(box->sizeHint());
    box->exec();
}

void CMessageBox::TipBox_Notes(const QString &text, QWidget *parent)
{

    QPixmap pixmap(SELECT_IAMGE);
    CMessageBox *box = new CMessageBox(parent);
    box->setWindowTitle(tr("Tip"));
    box->setTitle(tr("The Notes"));
    box->setImage(pixmap.scaled(QSize(26, 26)));
    box->setContentText(text);
    QPushButton *btnOk = new QPushButton(tr("Ok"));
    btnOk->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
    //btnOk->setStyleSheet(btStyleSheet);
    connect(btnOk, SIGNAL(clicked()), box, SLOT(close()));
    box->m_pUi->label_image->hide();
    btnOk->setFocusPolicy(Qt::NoFocus);
    box->addButton(btnOk);
    box->resize(box->sizeHint());
    box->exec();

    //QMessageBox *qmsg = new QMessageBox;
    //+qmsg->about(NULL, tr("The Notes"), text);
    //qmsg->move(QApplication::desktop()->width()/2, QApplication::desktop()->height()/2);

}

void CMessageBox::CriticalBox(const QString &text, QWidget *parent)
{
    QPixmap pixmap(CRITICAL_IMAGE);
    CMessageBox *box = new CMessageBox(parent);
    box->setWindowTitle(tr("Critical"));
    box->setTitle(tr("Critical"));
    box->setImage(pixmap.scaled(QSize(26, 26)));
    box->setContentText(text);
    QPushButton *btnOk = new QPushButton(tr("Ok"));
    btnOk->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
    //btnOk->setStyleSheet(btStyleSheet);
    connect(btnOk, SIGNAL(clicked()), box, SLOT(close()));
    btnOk->setFocusPolicy(Qt::NoFocus);
    box->addButton(btnOk);
    box->resize(box->sizeHint());
    box->exec();
}
void CMessageBox::TipBox(const QString &text, QWidget *parent)
{
    QPixmap pixmap(SELECT_IAMGE);
    CMessageBox *box = new CMessageBox(parent);
    box->setWindowTitle(tr("Tip"));
    box->setTitle(tr("Tip"));
    box->setImage(pixmap.scaled(QSize(26, 26)));
    box->setContentText(text);
    QPushButton *btnOk = new QPushButton(tr("Ok"));
    btnOk->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
    //btnOk->setStyleSheet(btStyleSheet);
    connect(btnOk, SIGNAL(clicked()), box, SLOT(close()));
    btnOk->setFocusPolicy(Qt::NoFocus);
    box->addButton(btnOk);
    box->resize(box->sizeHint());
    box->exec();
}

void CMessageBox::WarnBox(const QString &text, QWidget *parent)
{
    QPixmap pixmap(WARNING_IMAGE);
    CMessageBox *box = new CMessageBox(parent);
    box->setWindowTitle(tr("Warning"));
    box->setTitle(tr("Warning"));
    box->setImage(pixmap.scaled(QSize(26, 26)));
    box->setContentText(text);
    QPushButton *btnOk = new QPushButton(tr("Ok"));
    btnOk->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
    btnOk->setFocusPolicy(Qt::NoFocus);
    //btnOk->setStyleSheet(btStyleSheet);
    connect(btnOk, SIGNAL(clicked()), box, SLOT(close()));
    box->addButton(btnOk);
    box->resize(box->sizeHint());
    box->exec();
}

void CMessageBox::AboutBox(QWidget *parent)
{
    //QPixmap pixmap(ABOUT_IMAGE);
     //QPixmap pixmap(QString::fromUtf8(image));
     QPixmap pixmap(vclient_image.about_vCl_logo.c_str());
     //QPixmap pixmap(QString::fromUtf8(about.c_str()));
     //QPixmap pixmap2(VERTICAL_LINE_IMAGE);
     QPixmap pixmap2(vclient_image.vertical_line.c_str());

     char edition[64];
     memset(edition, 0, 64);
     sprintf(edition, "V%d.%d.%dBuild%04d", VER1, VER2, VER3, VER4);
     QString str_text;
#ifdef WIN32 //1
     str_text = tr("vClient for Windows");
#else
#ifdef VERSION_HXXY //2
     str_text = tr("Thinview");
#else
#ifdef VERSION_SUGON_2000 //3
     str_text = tr("Fronview2000");
#else
#ifdef VERSION_VCLASS
     str_text = tr("vClass");
#else
#ifdef VERSION_JSJQ
     memset(edition, 0, 64);
    //sprintf(edition, "V%d.%dBuild%d%d", VER1, VER2, VER3, VER4);
     strcpy(edition, "V1.0Build2014");
     str_text = tr("Jiang Su Sheng Jun Qu Hou Qin Cloud network office system");
#else
     str_text = tr("Fronview3000");
#endif
#endif
#endif //3
#endif //2
#endif //1

     CMessageBox *box = new CMessageBox(parent);
     //set style sheet
     QString styleSheet = "QWidget#widget_content{"
             "background-image:url(\":image/resource/image/skin_basic.png\");}";
     if(NULL != box->getUi())
         box->getUi()->widget_content->setStyleSheet(styleSheet);

     box->setWindowTitle(tr("About"));
     box->setTitle(tr("About"));
     box->setImage(pixmap.scaled(QSize(56, 67)));
#ifdef VERSION_FRONWARE
     str_text = str_text + tr("\nEdition: ") +  edition + tr("\n\nCopyright (C) 2014-2019 Fronware.\n");
     //tr("\n\nFromware software\n") +tr("Copyright © 2014-2019 Fronware.\n") + tr("All Rights Reserved.");
     box->setImage2(pixmap2.scaled(QSize(2, 100)), true);
#else
#ifdef VERSION_JSJQ
     str_text = str_text + tr("\nEdition: ") +  edition +
             tr("\n\nJiang Su Sheng Jun Qu Hou Qin Bu\n") +tr("Copyright (C) 2014-2020\n") +
             tr("Jiang Su Sheng Jun Qu Hou Qin Bu Lead's office of Information construction ");
#else
     str_text = str_text + tr("\nEdition: ") + edition + "\t\n\n" + tr("All Rights Reserved.");
     box->setImage2(pixmap2.scaled(QSize(2, 67)), true);
#endif
#endif
     box->setContentText(str_text);//(QString::fromLocal8Bit(text));
     //    QPushButton *btnOk = new QPushButton(tr("Ok"));
     //    btnOk->setFixedSize(PUSHBTN_WIDTH, PUSHBTN_HEIGHT);
     //    btnOk->setFocusPolicy(Qt::NoFocus);
     //    //btnOk->setStyleSheet(btStyleSheet);
     //    connect(btnOk, SIGNAL(clicked()), box, SLOT(close()));
     //    box->addButton(btnOk);
     //    box->resize(box->sizeHint());
     SysButton *btnClose = new SysButton("icon_close.png", tr("Close"), box);
     box->getUi()->horizontalLayout->addWidget(btnClose);
     connect(btnClose, SIGNAL(clicked()), box, SLOT(close()));
     //box->show();
     box->exec();

}


void CMessageBox::setSeatNumber()
{
    emit g_cmessagebox->on_signal_setSeatNumber();
}

void CMessageBox::sendSeatNumber()
{
    emit g_cmessagebox->on_signal_setSeatNumber_finished();
}

void CMessageBox::showSeatNumber()
{
    emit g_cmessagebox->on_signal_showSeatNumber();
}

void CMessageBox::showNotes()
{
    emit g_cmessagebox->on_signal_showNotes();
}

void CMessageBox::on_setSeatNumber()
{
    if (ACCEPTED == CMessageBox::messageBox_setseatNumber())
        g_cmessagebox->sendSeatNumber();
}

void CMessageBox::on_finish_setSeatNumber()
{
    std::string seat_number;
    seat_number = g_cmessagebox->setSeat.toStdString();
    LOG_INFO("To send seat number is %s", seat_number.c_str());
    g_ipcClient->sendWebsocketSeatNumber(seat_number);
}

void CMessageBox::on_showSeatNumber()
{
    g_cmessagebox->TipBox_seatNumber(g_cmessagebox->showSeat);
}

void CMessageBox::on_showNotes()
{
    g_cmessagebox->TipBox_Notes(codec->toUnicode(notes));
}
