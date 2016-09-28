#ifndef VCLIENTSTART_CONFIG_H
#define VCLIENTSTART_CONFIG_H

#include <QMainWindow>
#include <QRadioButton>
#include <QLineEdit>

namespace Ui {
class VclientStart_Config;
}

class VclientStart_Config : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit VclientStart_Config(QWidget *parent = 0);
    ~VclientStart_Config();

    int initwidget();

//    void paintEvent(QPaintEvent *);
//    void mouseMoveEvent(QMouseEvent *);
//    void mousePressEvent(QMouseEvent *);
//    void mouseReleaseEvent(QMouseEvent *);
public slots:
    int on_pushBtn_ok_clieked();
private:
    Ui::VclientStart_Config *ui;
    QRadioButton *m_autoLaunch ;
    QRadioButton *m_manualLaunch ;
    QRadioButton *m_autoLogin;
    QLineEdit *m_lineEditIp, *m_lineEditHostName;
    QPixmap m_backgroundPixmap;
    QPoint m_pressPoint;
    bool m_isMove;
};

#endif // VCLIENTSTART_CONFIG_H
