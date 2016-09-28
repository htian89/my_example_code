#ifndef CSELFSERVICEDIALOG_H
#define CSELFSERVICEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>

namespace Ui {
class CSelfServiceDialog;
}

//ALLUNABLE:            All buttons are unable
//ALLOWCLOSE:           Can be closed and restarted -->not used
//ALLOWSTART:           only start button are enable
//ALLOW_CONNECT:        the desktop(system) is running, allow to connect to the desktop  --->not used
//CANNOT_CONNECT:       the desktop(system) is running, but can't connect to the desktop --->not used
//ALLOW_DISCONN:        the desktop(system) is running, has connect to the desktop
//CANNOT_DISCONN:       the desktop(system) is running, has connect to the desktop but do not allow to disconnect
enum SELF_SERVICE_STATUS{ALLUNABLE=-1, ALLOWCLOSE, ALLOWSTART, ALLOW_CONNECT, CANNOT_CONNECT, ALLOW_DISCONN, CANNOT_DISCONN};
class SysButton;
class CLabel;
class CSelfServiceUnit;
class QHBoxLayout;

class CSelfServiceDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CSelfServiceDialog(SELF_SERVICE_STATUS m_serviceStatus, QWidget *parent = 0);
    ~CSelfServiceDialog();
    void setSelfServiceStatus(SELF_SERVICE_STATUS status);

protected:
    void paintEvent(QPaintEvent *pe );
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);
signals:
    void on_signal_start();
    void on_signal_restart();
    void on_signal_shutdown(int);
    void on_signal_rotatePixmap();
public slots:
    void on_btnStart_clicked();
    void on_btnRestart_clicked();
    void on_btnClose_clicked();

private:
    void setEnabled_ex(bool bStartEnabled, bool bRebootEnbled, bool bShutdownEnabled);
    int setConnectBtn(SELF_SERVICE_STATUS status);
private:
    Ui::CSelfServiceDialog *ui;
    QPixmap *pixmap;
    SysButton *btnSatrt;
    SysButton *btnReboot;
    SysButton *btnShutdown;

    CLabel *m_labelStart;
    CLabel *m_labelReboot;
    CLabel *m_labelShutdown;

    CSelfServiceUnit* m_unitStart; //-->now it's meanning is not start the desktop(system), but connect(disconnect) to the desktop
    CSelfServiceUnit* m_unitReboot;
    CSelfServiceUnit* m_unitShutdown;

    SELF_SERVICE_STATUS m_serviceStatus;
    QHBoxLayout* m_btnLayout;
};

class CLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CLabel(const QString &text, QWidget *parent = 0);
    void setEnabled_ex(bool);

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *ev);

signals:
    void clicked();
private:
    bool m_bEnabled;

};

class CSelfServiceUnit:public QWidget
{
    Q_OBJECT
public:
    CSelfServiceUnit(QString picName, QString tipText, QString labelText, QWidget *parent = 0);
    void setEnabled_ex(bool enable);
    int setContents(QString picName, QString tipText, QString labelText);

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *ev);

signals:
    void clicked();

private:
    QPixmap pixmap;
    int imgWidth;
    int imgHeight;
    QLabel m_label;
    QLabel m_labelImg;
    //QPushButton m_btn;

    bool m_bEnabled;

};

#endif // CSELFSERVICEDIALOG_H
