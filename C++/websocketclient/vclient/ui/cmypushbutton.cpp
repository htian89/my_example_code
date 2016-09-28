#include "cmypushbutton.h"
#include <QPainter>
#include <QPaintEvent>

CMyPushButton::CMyPushButton(QWidget *parent) :
    QPushButton(parent)
{
    setStyleSheet("QPushButton{"
                  "color: #FFFFFF;"
                  "font: bold 12pt;"
                  "border-radius: 0px;"
                  "selection-color:white;}"
                  "QPushButton:hover{"
                  "color: #dddddd;}"
                  "QPushButton:pressed{"
                  "color: #f8f8f8;}");
}
