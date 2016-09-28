#include "cthread.h"
#include "chttp.h"
#include <QDebug>

THREAD_TYPE g_threadType;

CThread::CThread(QString ip, int port, QObject *parent) :
    QThread(parent),
    m_ip(ip),
    m_port(port)
{
    memset(m_filepath, 0, sizeof(m_filepath));
    memset(m_filename, 0, sizeof(m_filename));
}

void CThread::checkVersion()
{
    std::string version;
    Chttp http(m_ip.toStdString(), m_port);
    http.checkVersion(version);
    m_version = QString::fromStdString(version);
    emit on_signal_complete(m_version);
}
/**
 * @brief CThread::getClientInfo
 * @desc thread func : get the install package name and path; the param m_filesize now is ignore;
 */
void CThread::getClientInfo()
{
    Chttp http(m_ip.toStdString(), m_port);
    //the file path style: eg: client/fronview3000/FronView3000_v2.9.2Build0060.iso
    int errorcode = http.getClientInfo(m_filepath, m_filesize);
    if(errorcode == 0)
    {
        strcpy(m_filename, m_filepath);
        while(strstr(m_filename, "/") != NULL)
        {
            strcpy(m_filename ,strstr(m_filename, "/")+1);
        }
        m_filename[strlen(m_filename)] = '\0';
        // note the ui we will download package;
        emit on_signal_getClientInfo_finished(m_filename, m_filesize, errorcode);
        qDebug() << "m_filepath: " << m_filepath;
        download();
    }
    else
    {
        //note the ui something is wrong when get client info;
        emit on_signal_getClientInfo_finished(m_filename, m_filesize, errorcode);
    }
}
/**
 * @brief CThread::download
 * @desc call http to download install package to local;
 */
void CThread::download()
{
    Chttp http(m_ip.toStdString(), m_port);
    connect(&http, SIGNAL(on_signal_http_download_progress(const char *,double)), this, SLOT(on_slot_thread_download_progress(const char *,double)));
    int errorcode = http.download(m_filepath, m_filename, m_filesize);
    //note the ui  have fininshed download or maybe not;
    emit on_signal_download_finished(errorcode);
}
void CThread::on_slot_thread_download_progress(const char *monitor,double progress)
{
    //note the ui download progress
    emit on_signal_thread_download_progress(monitor,progress);
}
/**
 * @brief CThread::run
 * @desc enum control the thread func;
 */
void CThread::run()
{
    switch(g_threadType){
    case TYPE_CHECKVERSION:
        checkVersion();
        break;
    case TYPE_GETCLIENTINFO:
        getClientInfo();
        break;
    default:
        break;
    }
}
