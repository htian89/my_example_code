#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include "multiAccessesDialog.h"
#include "multiAccessesTableWidget.h"
#include "ui_multiAccessesDialog.h"
#include "networksettingdialog.h"
#include "cmessagebox.h"
#include "common.h"
#include "../imageconf.h"

multiAccessesDialog::multiAccessesDialog(NetWorkSettingDialog *parent) :
    QDialog(parent),
    ui(new Ui::multiAccessesDialog)
{
    ui->setupUi(this);
    setWindowFlags( Qt::FramelessWindowHint|Qt::WindowSystemMenuHint|Qt::WindowMinimizeButtonHint | Qt::Dialog);
    m_backgroundPixmap.load(IMAGE_PATH_SETTING_WINDOWS_BACKGROUND);
    setAttribute(Qt::WA_TranslucentBackground);
    sysClose = new SysButton("icon_close.png", tr("Close"), this);
    ui->topLayout->addWidget(sysClose);
    connect(sysClose, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *rightlayout = new QHBoxLayout;

    //ui->top_cornernap->setPixmap(QPixmap(CORNER_TOP_LEFT_IMG));
    ui->top_cornernap->setPixmap(QPixmap(vclient_image.corner_left_top.data()));
    ui->top_title->setText(tr("multiAccessesDialog"));
    m_multiAccessesTableWidget = new multiAccessesTableWidget(this);
    ui->mainLayout->addWidget(m_multiAccessesTableWidget);



    QPushButton *pushButtonAdd = new QPushButton;
    pushButtonAdd->setText(tr("Add"));
    pushButtonAdd->setObjectName("pushButtonAdd");
    QPushButton *pushButtonDel = new QPushButton;
    pushButtonDel->setObjectName("pushButtonDel");
    pushButtonDel->setText(tr("Del"));
    QPushButton *pushButtonOk = new QPushButton;
    pushButtonOk->setText(tr("Ok"));
    pushButtonOk->setObjectName("pushButtonOk");
    rightlayout->addStretch();
    rightlayout->addWidget(pushButtonAdd);
    rightlayout->addWidget(pushButtonDel);
    rightlayout->addWidget(pushButtonOk);

   ui->mainLayout->addLayout(rightlayout);
    m_caller = parent;


//    m_multiAccessesTableWidget = new multiAccessesTableWidget(this);
//    leftlayout->addWidget(m_multiAccessesTableWidget);

//    QPushButton *pushButtonAdd = new QPushButton;
//    pushButtonAdd->setText(tr("Add"));
//    pushButtonAdd->setObjectName("pushButtonAdd");
//    QPushButton *pushButtonDel = new QPushButton;
//    pushButtonDel->setObjectName("pushButtonDel");
//    pushButtonDel->setText(tr("Del"));
//    QPushButton *pushButtonOk = new QPushButton;
//    pushButtonOk->setText(tr("Ok"));
//    pushButtonOk->setObjectName("pushButtonOk");
//    rightlayout->addWidget(pushButtonAdd);
//    rightlayout->addWidget(pushButtonDel);
//    rightlayout->addWidget(pushButtonOk);
//    rightlayout->addStretch();

//    QHBoxLayout *mainLayout = new QHBoxLayout;
//    mainLayout->addLayout(leftlayout);
//    mainLayout->addLayout(rightlayout);
//    setLayout(mainLayout);
    m_caller = parent;

    QMetaObject::connectSlotsByName(this);
}

multiAccessesDialog::~multiAccessesDialog()
{
    delete ui;
}

void multiAccessesDialog::on_pushButtonAdd_clicked()
{
    if(m_multiAccessesTableWidget->rowCount() > 0 && m_multiAccessesTableWidget->item(m_multiAccessesTableWidget->rowCount()-1, 0)->text().isEmpty()){
        CMessageBox::TipBox(tr("mush have writen at least server IP."), this);
        return;
    }
    int newRowCount = 0;
    newRowCount = m_multiAccessesTableWidget->rowCount() + 1;
    m_multiAccessesTableWidget->setRowCount(newRowCount);
    qDebug() << "newRowCount after adding" << m_multiAccessesTableWidget->rowCount() << endl;
    for (int column = 0; column < m_multiAccessesTableWidget->columnCount(); column++) {
        m_multiAccessesTableWidget->setItem(m_multiAccessesTableWidget->rowCount() - 1, column, new QTableWidgetItem);
        qDebug() << "create new item succeed " <<  m_multiAccessesTableWidget->rowCount() - 1 << ':' << column << endl;
    }
}

void multiAccessesDialog::on_pushButtonDel_clicked()
{
    if(m_multiAccessesTableWidget->rowCount() >0){
        if (-1 == m_multiAccessesTableWidget->m_SelectedRow ) {
            CMessageBox::TipBox(tr("Please choose the row you want to delete first."), this);
        }
        m_multiAccessesTableWidget->removeRow(m_multiAccessesTableWidget->m_SelectedRow);
        m_multiAccessesTableWidget->m_SelectedRow = -1;
    }
//    if (m_multiAccessesTableWidget->rowCount() > 1) {
//        if (-1 == m_multiAccessesTableWidget->m_SelectedRow ) {
//            CMessageBox::TipBox(tr("Please choose the row you want to delete first."), this);
//        }
//        m_multiAccessesTableWidget->removeRow(m_multiAccessesTableWidget->m_SelectedRow);
//        m_multiAccessesTableWidget->m_SelectedRow = -1;
//    } else {
//        CMessageBox::TipBox(tr("mush have at least one vAccess IP."), this);
//    }
}

void multiAccessesDialog::on_pushButtonOk_clicked()
{
    if (m_multiAccessesTableWidget->checkValidation() == 0){
        qDebug() << "checkValidation return 0" << endl;
        m_multiAccessesTableWidget->writeFile();
        m_caller->refreshAccessIpList();
        this->close();
    }
}

void multiAccessesDialog::mouseMoveEvent(QMouseEvent *_qm)
{
    if(_qm->buttons() == Qt::LeftButton && m_isMove)
        move(_qm->globalPos() - m_pressPoint);
}

void multiAccessesDialog::mousePressEvent(QMouseEvent *_qm)
{
    m_isMove = true;
    m_pressPoint = _qm->pos();
}

void multiAccessesDialog::mouseReleaseEvent(QMouseEvent *)
{
    m_isMove = false;
}

void multiAccessesDialog::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setBrush(Qt::NoBrush);
//    QPen pen;
//    QColor color(0, 0, 0, 50);
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
