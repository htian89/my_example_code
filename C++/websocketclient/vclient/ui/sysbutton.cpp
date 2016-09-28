#include "sysbutton.h"
#include <QPainter>
#include <QDebug>

SysButton::SysButton(QString picName, QString tipText, QWidget *parent) :
    QPushButton(parent),
    m_allowPaint(true)
{
    pixmap.load(":image/resource/image/"+picName);
	setWindowOpacity(0);
    setFlat(true);
    btnWidth = pixmap.width()/3;
    btnHeight = pixmap.height();
	setFixedSize(btnWidth, btnHeight);
	setToolTip(tipText);
	status = NORMAL;
    setFocusPolicy(Qt::NoFocus);
}

void SysButton::setPixmap(QString picName)
{
    pixmap.detach();
    pixmap.load(":image/resource/image/"+picName);
    update();
}

void SysButton::setEnabled_ex(bool enable)
{
    setEnabled(enable);
    if(enable)
        m_allowPaint = true;
    else
        m_allowPaint = false;
}

void SysButton::enterEvent(QEvent *)
{
    status = ENTER;
	update();
}

void SysButton::mousePressEvent(QMouseEvent *)
{
    status = PRESS;
	update();
}

void SysButton::mouseReleaseEvent(QMouseEvent *)
{
    status = ENTER;
	update();
	emit click();
}

void SysButton::leaveEvent(QEvent *)
{
    status = NORMAL;
	update();
}

void SysButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if(m_allowPaint)
        painter.drawPixmap(rect(), pixmap.copy(btnWidth*status, 0, btnWidth, btnHeight));
    else
        painter.drawPixmap(rect(), pixmap.copy(btnWidth*NORMAL, 0, btnWidth, btnHeight));
}
