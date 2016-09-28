#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H
#include <QObject>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include "myudpsocket.h"
#include "updateinfoview.h"
class AutoUpdate : public QObject
{
    Q_OBJECT
public:
    explicit AutoUpdate(const char *serverAdress, const char* port, QObject *parent = 0);
    ~AutoUpdate();

    void checkVersion();
    int compareVersion(QString currentStr, QString newStr);

    void processUPdateResult(UPDATERESULT updateResult, const QString &datagrams);
    void update_progress(const QStringList &spData);

private slots:
    void on_checkversion_finished(QString);
    void showOnTextArea(QByteArray);
    void on_slot_operate_reboot();
    void on_slot_reboot_clicked();
    void on_slot_cancel_clicked();
private:
    const char* m_serverAddress;
    const char* m_port;
    MyUdpSocket *m_udpSocket;
    volatile UPDATERESULT m_updateResult;
    volatile REBOOTSTATUE m_rebootResult;
    int m_iTimeout;
    QMessageBox *m_msgBox;
    QPushButton *m_rebootBtn, *m_cancelBtn;
    QTimer m_timer;
    QString m_text;
};


#endif // AUTOUPDATE_H
