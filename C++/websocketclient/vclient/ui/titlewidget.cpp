#include "titlewidget.h"
#include "sysbutton.h"
#include <QtGui>
#include <log.h>

TitleWidget::TitleWidget(TITLEBARTYPE type, QString titleText, QWidget *parent) :
    QWidget(parent),
    btnClose(NULL),
    btnMinimize(NULL),
    btnMaxmize(NULL),
    windowState(0)
{
//	QPalette palette;
//    palette.setBrush(QPalette::Window, QBrush(QColor(0,0,0,0)));
//	setPalette(palette);
//	setAutoFillBackground(true);
    setFocusPolicy(Qt::NoFocus);
    setWindowOpacity(0);
	QLabel *titleLabel= new QLabel(titleText);
    parentWidget = parent;
    switch(type)
    {
    case ALLSYSBUTTON:
        btnClose = new SysButton("icon_close.png", tr("Close"));
        btnMinimize = new SysButton("icon_minimize.png",tr("Minimize"));
        btnMaxmize = new SysButton("icon_exchange.png", tr("Maxmize"));
        break;
    case NOMAXMIZE:
        btnClose = new SysButton("icon_close.png", tr("Close"));
        btnMinimize = new SysButton("icon_minimize.png",tr("Minimize"));
        break;
    case NOMINIMIZE:
        btnClose = new SysButton("icon_close.png", tr("Close"));
        btnMaxmize = new SysButton("icon_exchange.png", tr("Maxmize"));
        break;
    case ONLYCLOSE:
        btnClose = new SysButton("icon_close.png", tr("Close"));
        break;
    case ONLYMINIMIZE:
         btnMinimize = new SysButton("icon_minimize.png",tr("Minimize"));
         break;
    default:
        break;
    }
    if(btnClose)
        connect(btnClose, SIGNAL(clicked()),parentWidget,SLOT(close()));
    if(btnMinimize)
        connect(btnMinimize, SIGNAL(clicked()), parentWidget, SLOT(showMinimized()));
    if(btnMaxmize)
        connect(btnMaxmize, SIGNAL(clicked()), this, SLOT(on_btnMaxmize_clicked()));

    mainLayout = new QHBoxLayout;//QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(titleLabel);
    titleLabel->setContentsMargins(5,0,0,0);
//    mainLayout->addStretch();
    if(btnMinimize)
        mainLayout->addWidget(btnMinimize);
    if(btnMaxmize)
        mainLayout->addWidget(btnMaxmize);
    if(btnClose)
        mainLayout->addWidget(btnClose);
    mainLayout->setContentsMargins(0,0,0,0);
	setLayout(mainLayout);
    setFixedHeight(28);
    setFixedWidth(parent->width()-5);
	isMove = false;
}

TitleWidget::~TitleWidget()
{
	delete btnClose;
}

int TitleWidget::setLayout_title(int left, int top, int right, int bottom)
{
    if(NULL != mainLayout)
       mainLayout->setContentsMargins(left, top, right, bottom);
    return 0;
}

int TitleWidget::setImage(const QString picName, enum TITLE_IMAGE_TYPE imageType)
{
    int iRet = -1;
    if(imageType == MINIMIZE)
    {
        if(NULL != btnMinimize)
        {
            btnMinimize->setPixmap(picName);
            iRet = 0;
        }
        else
            LOG_ERR("%s", "NULL == btnMinimize");
    }
    else if(imageType == CLOSE)
    {
        if(NULL != btnClose)
        {
            btnClose->setPixmap(picName);
            iRet = 0;
        }
        else
            LOG_ERR("%s", "NULL == btnClose");
    }
    else if(imageType == MAXMIZE)
    {
        if(NULL != btnMaxmize)
        {
            btnMaxmize->setPixmap(picName);
            iRet = 0;
        }
        else
            LOG_ERR("%s", "NULL == btnMaxmize");
    }
    else
    {
        LOG_ERR("unknown image type:%d", int(imageType));
    }
    return iRet;
}

void TitleWidget::mousePressEvent(QMouseEvent *e)
{
	if (e->x()+50 >= this->width())
		return;
	pressedPoint = e->pos();
	isMove = true;
}

void TitleWidget::mouseMoveEvent(QMouseEvent *e)
{
	if( (e->buttons()&Qt::LeftButton) && isMove)
	{
        QPoint nowParPoint=parentWidget->pos();
		nowParPoint.setX(nowParPoint.x()+e->x()-pressedPoint.x());
		nowParPoint.setY(nowParPoint.y()+e->y()-pressedPoint.y());
        parentWidget->move(nowParPoint);
	}
	e->ignore();
}

void TitleWidget::mouseReleaseEvent(QMouseEvent *)
{
	if(isMove)
		isMove = false;
}

void TitleWidget::on_btnMaxmize_clicked()
{
    if(parentWidget==NULL)
        qDebug("NULL");
    if(windowState == 0)
    {
        parentWidget->showMaximized();
        windowState = 1;
    }
    else
    {
        parentWidget->showNormal();
        windowState = 0;
    }
}
