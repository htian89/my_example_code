#ifndef UPDATEINFOVIEW_H
#define UPDATEINFOVIEW_H

#include <QMainWindow>
#include <QColor>
#include "myudpsocket.h"

enum UPDATERESULT{FAILED = -1, UNFINISHED, FINISHED};
enum BUTTONSTATUE{UPDATE, REBOOT};
enum REBOOTSTATUE{UNKNOWN, REBOOTOK, REBOOTNO};

namespace Ui {
class UpdateInfoView;
}

class UpdateInfoView : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit UpdateInfoView(const char *serverAddr, const char  *port, QWidget *parent = 0);
    ~UpdateInfoView();
    void appendText(const QString &text);
    void processUPdateResult(UPDATERESULT updateResult, const QString &datagrams);
    void update_progress(const QStringList &spData);
    void setInsertTextColor(QColor color);
    int  checkVersion();
    int compareVersion(QString currentVersion, QString newVersion);
    
private slots:

    void on_pushButton_update_clicked();

    void showOnTextArea(QByteArray);

    void on_pushButton_reboot_clicked();

    void on_checkVersion_finish(QString);

private:
    Ui::UpdateInfoView *ui;
    MyUdpSocket *udpSocket;
    volatile UPDATERESULT m_updateResult;
    BUTTONSTATUE buttonStatus;
    QByteArray m_serverAddr;
    int m_port;
};

#endif // UPDATEINFOVIEW_H
