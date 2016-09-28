#include "cmytabwidget.h"
#include <QTabBar>
#include <QFile>

CMyTabWidget::CMyTabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    setWindowOpacity(0);
//    setDocumentMode(true);
    QTabBar *tab_bar = tabBar();
    this->setFocusPolicy(Qt::NoFocus);

    QString string ="QTabBar::tab{"
            "font: bold 15px;"
            "color: rgb(0, 0, 0);"
            "border: 0px solid #C4C4C3;"
            "border-bottom-color: #C2C7CB;"
            "border-top-left-radius: 2px;"
            "border-top-right-radius: 2px;"
            "min-width: 80px;"
            "min-height: 18px;"
            "padding:5px;}"
            "QTabBar::tab:selected{"
            "font: bold 15px;"
            "border-color: #9B9B9B;"
            "border-bottom-color: #C2C7CB;"
            "background-image: url(\":image/resource/image/skin_basic.png\");}"
            "QTabBar::tab:!selected{"
            "font: bold 15px;"
            "border-radius:4px;"
            "margin-top: 0px;}"
            "QTabBar::tab:first{"
            "margin-left: 50px;"
            "}";

    /*"QDialog{"
    "border:1px solid #e9e9e9;"
    "border-radius:8px;}"*/
    /*QString string ="QTabBar::tab{"
            "color: rgb(255, 255, 255);"
            "border: 0px solid #C4C4C3;"
            "border-bottom-color: #C2C7CB;"
            "border-top-left-radius: 0px;"
            "border-top-right-radius: 0px;"
			"min-width: 3ex;"
			"min-height: 18px;"
			"padding: 5px;}"
            "QTabBar::tab:selected{"
            "border-color: #9B9B9B;"
            "border-bottom-color: #C2C7CB;"
            "background-image: url(\":image/resource/image/skin_basic.png\");}"
            "QTabBar::tab:!selected{"
            "margin-top: 1px;}";*/

    tab_bar->setStyleSheet(string);

    QPalette pal = palette();
    pal.setBrush(QPalette::WindowText, QColor(QRgb(0)));//QColor(QRgb(0xFFFFFF)));//
    setPalette(pal);
}

QTabBar* CMyTabWidget::getTabBar()
{
    return tabBar();
}
