#include <QMouseEvent>
#include <QPainter>
#include "cselectdialog.h"
#include "ui_cselectdialog.h"
#include "../config.h"
#include "../imageconf.h"

#define IMAGE_PATH_BACKGROUND  ":image/resource/image/skin_expand.png"

CSelectDialog::CSelectDialog(const QString str_text0, const QString str_text1, const QString str_tips, \
                             const QString& str_title, int selection, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSelectDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint|Qt::WindowMinimizeButtonHint | Qt::Dialog);
    //setWindowIcon(QIcon(WINDOWS_ICON));
    setWindowIcon(QIcon(WINDOWS_IMG.data()));
//    QPalette pal = palette();
//    pal.setBrush(QPalette::Window, QBrush(QPixmap(IMAGE_PATH_BACKGROUND)));
//    setPalette(pal);
    setAttribute(Qt::WA_TranslucentBackground);
    m_backgroundPixmap.load(IMAGE_PATH_BACKGROUND);
    ui->widget->setStyleSheet("background-color: rgb(234,234,234);");
    //ui->label_corner->setPixmap(QPixmap(CORNER_TOP_LEFT_IMG));
    ui->label_corner->setPixmap(QPixmap(vclient_image.corner_left_top.data()));
    m_str_text0 = str_text0;
    m_str_text1 = str_text1;
    m_str_tips = str_tips;
    m_str_title = str_title;
    m_seletedItem = selection;

    if(0 == m_seletedItem)
    {
        ui->radioButton_1->setChecked(true);
        ui->radioButton_2->setChecked(false);
    }
    else
    {
        ui->radioButton_1->setChecked(false);
        ui->radioButton_2->setChecked(true);
    }
    setWindowTitle(tr("Tip"));
    ui->label_title->setText(m_str_title);
    ui->label_tips->setText(m_str_tips);
    ui->radioButton_1->setText(m_str_text0);
    ui->radioButton_2->setText(m_str_text1);
    ui->pushButton_ok->setStyleSheet(STYLE_SHEET_PUSHBTN);

    connect(ui->pushButton_ok, SIGNAL(clicked()), this, SLOT(on_pushbtnClicked()));
}

CSelectDialog::~CSelectDialog()
{
    disconnect(ui->pushButton_ok, SIGNAL(clicked()), this, SLOT(on_pushbtnClicked()));
    delete ui;
}

int CSelectDialog::getSelection()
{
    return m_seletedItem;
}

void CSelectDialog::on_pushbtnClicked()
{
    if(ui->radioButton_1->isChecked())
        m_seletedItem = 0;
    else if(ui->radioButton_2->isChecked())
        m_seletedItem = 1;
    close();
}

void CSelectDialog::mouseMoveEvent(QMouseEvent *_qm)
{
    if(_qm->buttons() == Qt::LeftButton && m_isMove)
        move(_qm->globalPos() - m_pressPoint);
}

void CSelectDialog::mousePressEvent(QMouseEvent *_qm)
{
    m_isMove = true;
    m_pressPoint = _qm->pos();
}

void CSelectDialog::mouseReleaseEvent(QMouseEvent *)
{
    m_isMove = false;
}
void CSelectDialog::paintEvent(QPaintEvent *)
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
    paint.setBrush(QBrush(m_backgroundPixmap));
    paint.setPen(Qt::transparent);
    paint.drawRoundRect(QRectF(0,0,width(),height()),0,0);
//    paint.drawRoundRect(QRectF(0, 0, width()-6, height()-6), 3, 3);
}

