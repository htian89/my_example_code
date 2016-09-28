#ifndef CTHREAD_H
#define CTHREAD_H

#include <QThread>

class CThread : public QThread
{
    Q_OBJECT
public:
    explicit CThread(QString ip, int port, QObject *parent = 0);
    void run();

    void checkVersion();
    
signals:
    void on_signal_complete(QString);
    
public slots:

private:
    QString m_ip;
    int m_port;
    QString m_version;
    
};

#endif // CTHREAD_H
