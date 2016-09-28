#include "myudpsocket.h"
#include <QProcess>

MyUdpSocket::MyUdpSocket(QByteArray serverAddr)
{
    if(!serverAddr.isEmpty())
        strcpy(this->serverAddr, serverAddr.data());
    initUdpSocket();
    startServer();
}

MyUdpSocket::~MyUdpSocket()
{
    if(udpSocket != NULL)
        delete udpSocket;
}

void MyUdpSocket::startServer()
{
    char cmd[512] ;
    char sourceUpdateAddr[256];
    memset(sourceUpdateAddr, 0, 256);
    strcat(sourceUpdateAddr, "http://");
    strcat(sourceUpdateAddr, serverAddr);
    strcat(sourceUpdateAddr, "/FronViewUpdate/");
    memset(cmd, 0, sizeof(cmd));
    strcat(cmd, "python /etc/systemupdate/system_update.py ");
    strcat(cmd, sourceUpdateAddr);
//    qUpdateDebug("start server: %s", cmd);
    QProcess *process = new QProcess;
    process->start(cmd);
}

void MyUdpSocket::initUdpSocket()
{
//    qUpdateDebug("initilize udpSocket...");
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(port);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readDatagrams()));
}

void MyUdpSocket::readDatagrams()
{
//    qUpdateDebug("recv a params...");
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagrams;
        datagrams.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagrams.data(), datagrams.size());
        emit sendDatagramsToView(datagrams);
    }
}
