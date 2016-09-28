#ifndef IPCITALC_H
#define IPCITALC_H
#ifdef WIN32
#include  <winsock2.h>
#include "../common/cthread.h"
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

//#include "cJSON.h"
#include "../common/ds_session.h"
#include <QObject>
#include "mxml.h"
#include "base64.h"
#include "../backend/csession.h"
#include "../common/log.h"
#include "../common/common.h"
#include "../common/errorcode.h"
#include "../common/cthread.h"
#include "cmessagebox.h"

class IpcItalc : public QObject
{
    Q_OBJECT
public:
    IpcItalc();
    ~IpcItalc();
    int ipcSendMsg(const char *msg, const int dataLen);
    int ipcRecvMsg(char *msg);
    void ipcClose();
    inline int getSockfd(){return m_sockfd;}
    inline void setConnected(bool isConnected){m_isConnected = isConnected;}
    inline sockaddr_in getSockAddr(){return m_addr;}
    int sendMonitorsInfo(LIST_MONITORS_INFO &pListMonitorsInfo);
    int sendUserOrganizations(USERORGANIZATIONS &userorganizations);
    static void *ipcItalcProcessMsg(void *);
    static void *ipcItalcRequestMonitorsInfo(void *);
    static void *ipcItalcListUserReource(void *);
    static void *ipcItalcHello(void *arg);
    static void *getuserOrganizations(void *);
    static void *addOrganization(void *);
    static void *deleteOrganization(void *);
    static void *updateOrganization(void *);
    static void *moveOrganizationUsers(void *);
    static void *addOrganizationUsers(void *);
    static void *deleteOrganizationUsers(void *);
    static void *getOrganizationDetail(void *);
    static void *getRoles(void *);
    static void *addRole(void *);
    static void *deleteRole(void *);
    static void *updateRole(void *);
    static void *addroleTousers(void *);
    static void *deleteroleFromUsers(void * );
    static void *getuserprivileges(void *);
    static void *getprivileges(void *);
    static void *addPrivileges(void *);
    static void *deletePrivileges(void *);
    static void *setSeatNumbers(void *);
    int monitorinfoChange();

protected:
    void _initialize();
signals:
    void on_connect_monitor(CONNECT_MONITOR&);
    void on_disconnect_monitor(DISCONNECT_MONITOR&);
    void on_signal_addTerminalList(std::vector<APP_LIST> );
    void on_signal_deleteTerminalList(std::vector<APP_LIST> );

public slots:
    void sendAddTerminalList(std::vector<APP_LIST>);
    void sendDeleteTerminalList(std::vector<APP_LIST>);
    static void* on_slot_connect_monitor(void*);
    static void* on_slot_disconnect_monitor(void*);
private:
    sockaddr_in m_addr;
#ifdef WIN32
    SOCKET  m_sockfd;
    THREAD_HANDLE m_threadHandle;
#else
    int m_sockfd;
#endif
    static bool m_isConnected;
    static CSession *m_pSession;
    static IpcItalc *m_ipcItalc;
    LIST_MONITORS_INFO *m_listMonitorsInfo;
    std::vector<APP_LIST> m_appbaklist;
    string m_strMonitorsInfo;
};

#endif // IPCITALC_H
