#ifndef CUSBMAP_H
#define CUSBMAP_H
#include <string>
#include "../common/ds_vclient.h"
//#include "../common/ds_session.h"
#include "../common/ds_launchapp.h"

#ifdef _WIN32
#include <windows.h>
#endif

class CProcessOp;
class CSession;

class CUsbMap
{
public:
    CUsbMap();
    ~CUsbMap();
    int launchUsbMap(const APP_LIST& appInfo, const RESOURCE_PARAMETERS &resParam, const NETWORK& netWork,
                     const LAUNCH_RESOURCE_DATA& resData);
    static void  *waitUsbMap(void *);
//    static void waitUsbMap(int signo);
    int vaccessCloseChannel();
    int quit();

#ifdef WIN32
private:
    int writeToSlot(const std::string strSlotName, const std::string str_data);
    int writeToSlot(const HANDLE hd, const std::string str_data);
    int readFromSlot(const HANDLE hd, std::string& str_data_read);
#endif

private:
    CSession* m_pSession;
    CProcessOp* m_pProcOp_usbMap;
    std::string m_desktopIp;
    bool b_hasMapped;
    static int m_iExitCode;
#ifdef WIN32
    HANDLE m_hdSlotVclient;
    HANDLE m_hdSlotFusb;
#endif

};

#endif // CUSBMAP_H
