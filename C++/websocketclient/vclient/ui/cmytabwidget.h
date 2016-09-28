#ifndef CMYTABWIDGET_H
#define CMYTABWIDGET_H

#include <QTabWidget>

class CMyTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit CMyTabWidget(QWidget *parent = 0);
    QTabBar* getTabBar();
    
signals:
    
public slots:
    
};

#endif // CMYTABWIDGET_H
