#ifndef CFRONTENDBASE_H
#define CFRONTENDBASE_H
#include <string>
#include "../common/ds_vclient.h"
#include "../common/ds_session.h"
#include "../common/ds_launchapp.h"
#include "../common/cevent.h"
#include "../common/cmutexop.h"
#include "cusbmap.h"

class CProcessOp;
class CSession;
class CFrontEndBase
{
public:
    CFrontEndBase(const LAUNCH_CALLBACK_INFO callBackInfo);
    virtual ~CFrontEndBase();
    int runInSubProcAndWait(std::string appName, std::string cmdParam, APP_LIST appInfo);
    int queryVaccess(const APP_LIST& appInfo);
    int vaccessShutDownResource(int isRelease = 0);
    int vaccessListUserResource(LIST_USER_RESOURCE_DATA &userResData);
    int vaccessAttachDisk(const APP_LIST &appInfo, const VIRTUALDISK& vDisk);
    int AttachVDisk(const APP_LIST &appInfo, bool bAttach, const VIRTUALDISK& vDisk);
//    virtual int formCmdParam_win() = 0;
//    virtual int formCmdParam_unix() = 0;
    virtual LAUNCH_TYPE LaunchType() const = 0;
    virtual int Launch() = 0;
    virtual int quitProc(); //used to stop rdp or fap process
    int  release();
    char *getCmdSystem(const char *cmd);
    int procStatus();
    int setForeground(LAUNCH_TYPE, const char *);

private:
    CProcessOp* m_pProcOp;
    CMutexOp m_mutex;
public:
    CSession* m_pSession;
    RESOURCE_PARAMETERS m_resParam;
    LAUNCH_RESOURCE_DATA m_resData;
    LAUNCH_CALLBACK_INFO m_pCallBackInfo;
    bool m_bProcessIsRunning;
    bool m_bNeedMapUsb;
    bool m_bNeedMapFileSystem;
    bool m_bUserRequestQuit;
    CEvent m_event;
    CUsbMap *m_usbMap;

    bool m_hasReleased;
    CEvent m_eventWaitRelease;
};

#endif // CFRONTENDBASE_H
