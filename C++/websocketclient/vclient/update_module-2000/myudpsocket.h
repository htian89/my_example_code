#ifndef MYUDPSOCKET_H
#define MYUDPSOCKET_H

#include <QUdpSocket>
//#include <QDebug>

#define qUpdateDebug(format, ...) \
    qDebug("[LINUX_UPDATE] "format" File:%s, Line:%d, Function:%s", ##__VA_ARGS__, __FILE__, __LINE__, __FUNCTION__)

const quint16 port = 51229;

class MyUdpSocket : public QObject
{
    Q_OBJECT
public:
    MyUdpSocket(QByteArray serverAddr);
    ~ MyUdpSocket();
    void initUdpSocket();
    void startServer();
signals:
    void sendDatagramsToView(QByteArray);
public slots:
    void readDatagrams();

private:
    QUdpSocket *udpSocket;
    char serverAddr[128];

};

#endif // MYUDPSOCKET_H
