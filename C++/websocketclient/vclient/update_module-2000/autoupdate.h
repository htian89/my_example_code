#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H
#include <QObject>
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
private:
    const char* m_serverAddress;
    const char* m_port;
    MyUdpSocket *m_udpSocket;
    volatile UPDATERESULT m_updateResult;
};


#endif // AUTOUPDATE_H
