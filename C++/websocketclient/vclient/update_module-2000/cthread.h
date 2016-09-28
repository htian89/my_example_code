#ifndef CTHREAD_H
#define CTHREAD_H
#include <QObject>
#include <QThread>
enum thread_type{TYPE_UNKNOWN, TYPE_CHECKVERSION, TYPE_GETCLIENTINFO, TYPE_DOWNLOADPROGRESS};
typedef enum thread_type THREAD_TYPE;
class CThread : public QThread
{
    Q_OBJECT
public:
    explicit CThread(QString ip, int port, QObject *parent = 0);
    void run();

    void checkVersion();
    void getClientInfo();
    void download();
    void downloadProgress();
    
signals:
    void on_signal_complete(QString);
    void on_signal_getClientInfo_finished(QString, int, int);
    void on_signal_download_finished(int);
    void on_signal_thread_download_progress(const char *,double);
public slots:
    void on_slot_thread_download_progress(const char *, double);
private:
    QString m_ip;
    int m_port;
    QString m_version;
    char m_filepath[512];
    char m_filename[512];
    int m_filesize;
};

#endif // CTHREAD_H
