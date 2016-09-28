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

VclientStart_Config::VclientStart_Config(QWidget*parent) :
    QMainWindow(parent),
    ui(new Ui::VclientStart_Config)
{
    ui->setupUi(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *titleLayout   = new QHBoxLayout;
    QHBoxLayout *contentLayout = new QHBoxLayout;
    QGridLayout *gridLayout = new QGridLayout;
    QHBoxLayout *bottomLayout  = new QHBoxLayout;

    QLabel *title_png = new QLabel;
    title_png->setPixmap(QPixmap(CORNER_TOP_LEFT_IMG));
    QLabel *title_text = new QLabel(tr("Config login vclient"));
    titleLayout->addWidget(title_png);
    titleLayout->addWidget(title_text);
    titleLayout->addStretch();


    QRadioButton *autoLaunch = new QRadioButton;
    QRadioButton *manualLaunch = new QRadioButton;
    gridLayout->addWidget(autoLaunch, 0,0);
    gridLayout->addWidget(manualLaunch, 1,0);
    contentLayout->addLayout(gridLayout);

    autoLaunch->setText(tr("Auto launch desktop, don't need login and configure."));
    manualLaunch->setText(tr("Manual login vclient, need configure the vAccess ip."));



    QDialogButtonBox *requestButtonBox = new QDialogButtonBox();
    QPushButton *pushBtn_ok = new QPushButton(tr("OK"));
    QPushButton *pushBtn_cancel = new QPushButton(tr("Cancel"));
    pushBtn_ok->setFixedSize(70, 25);
    pushBtn_cancel->setFixedSize(70, 25);
    requestButtonBox->addButton(pushBtn_ok,QDialogButtonBox::AcceptRole);
    requestButtonBox->addButton(pushBtn_cancel,QDialogButtonBox::RejectRole);
    pushBtn_ok->setStyleSheet(STYLE_SHEET_PUSHBTN);
    pushBtn_cancel->setStyleSheet(STYLE_SHEET_PUSHBTN);

    bottomLayout->addStretch();
    bottomLayout->addWidget(requestButtonBox);



    mainLayout->addLayout(titleLayout);
    mainLayout->addLayout(contentLayout);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);
}

VclientStart_Config::~VclientStart_Config()
{
    delete ui;
}
