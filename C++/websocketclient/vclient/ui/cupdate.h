#ifndef CUPDATE_H
#define CUPDATE_H
#include <string>
#include <QThread>
#include "../common/ds_vclient.h"
#include "../common/ds_settings.h"

#define UPDATE_VCLIENT_PATH "updateVClient.exe"
#define AUTO_UPDATE_VCLIENT_SERVICE_APP "updateService.exe"
#define AUTO_UPDATE_VCLIENT_SERVICE_NAME "updatevclient"

class CProcessOp;
class CUpdate:public QThread
{
public:
    CUpdate(const SETTINGS_VCLIENT&);
    ~CUpdate();
    void run();
    static int stopAutoUpdateService();
    static int setAutoUpdate(bool bAutoUpdate);
    static int getAutoUpdateServiceStatus();

private:
    SETTINGS_VCLIENT m_vclientSetting;
    CProcessOp* m_pProc;
    int m_iRet;
    bool m_hasFinished;
    //std::string m_fileName;

};

#endif // CUPDATE_H
