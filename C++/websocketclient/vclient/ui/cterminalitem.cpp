#include "cterminalitem.h"
#include "sysbutton.h"
#include "cmessagebox.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QDesktopWidget>
#include <QMutex>
#include "../common/log.h"
#include "../common/ds_launchapp.h"
#include "../common/cconfiginfo.h"
#include "../backend/csession.h"

CTerminalItem::CTerminalItem(QWidget *parent) :
    QCheckBox(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);
    setMouseTracking(true);
    connect(this,SIGNAL(stateChanged(int)), this, SLOT(on_stateChanged(int)));
}
void CTerminalItem::setTerminalName(QString name, QString ip)
{
    QString text = name + ":\r";
    text = text + ip;
    setText(text);
}

void CTerminalItem::on_stateChanged(int state)
{
    emit on_stateChanged_signal(m_itemDada.uuid, state);
}

CTerminalItem::~CTerminalItem()
{

}
