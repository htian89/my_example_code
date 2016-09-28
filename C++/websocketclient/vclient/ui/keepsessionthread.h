#ifndef KEEPSESSIONTHREAD_H
#define KEEPSESSIONTHREAD_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include "../backend/csession.h"

class RefreshListThread : public QThread
{
    Q_OBJECT
public:
    explicit RefreshListThread();
    ~RefreshListThread();
    void setStop(bool stop);
    void run();

signals:
    void on_signal_update_resourceList(LIST_USER_RESOURCE_DATA stAppList,
                                       GET_USER_INFO_DATA vstVirtualDisks,
                                       GET_RESOURCE_PARAMETER resParam_out);
    void on_signal_update_failed(int errorCode, int dType);

public slots:
    void on_appIsActivatedSlot(int state);
private:
    bool m_stop;
    CSession *m_pSession;
    QWaitCondition m_waitCondition;
    QMutex m_mutex;
    int m_iAppIsActivated;
    time_t m_time;
    bool m_hasDisconnectSignalAppIsActivated;
};

#endif // KEEPSESSIONTHREAD_H
