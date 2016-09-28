#include "cthread.h"
#include "chttp.h"

CThread::CThread(QString ip, int port, QObject *parent) :
    QThread(parent),
    m_ip(ip),
    m_port(port)
{
}

void CThread::checkVersion()
{
    std::string version;
    Chttp http(m_ip.toStdString(), m_port);
    http.checkVersion(version);
    m_version = QString::fromStdString(version);
    emit on_signal_complete(m_version);
}

void CThread::run()
{
    checkVersion();
}
