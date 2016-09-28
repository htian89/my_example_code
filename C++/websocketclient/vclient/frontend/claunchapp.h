#ifndef CLAUNCHAPP_H
#define CLAUNCHAPP_H
#include <map>
#include <string>
#include "../common/ds_launchapp.h"
#include "../common/cmutexop.h"


class CLaunchApp
{
public:
    CLaunchApp();
    ~CLaunchApp();
    int launchRdp(const PARAM_LAUNCH_COMMON_IN& stLaunchCommon, const LAUCH_RDP& stLaunchRdp);
    int launchFap(const PARAM_LAUNCH_COMMON_IN& stLaunchCommon, const LAUCH_FAP& stLaunchFap);
    int launchTerminal(const PARAM_LAUNCH_COMMON_IN& stLaunchCommon, const LAUNCH_TERMINAL& stLaunchTerminal);
    //iReloadDisk: 0: do not reload Disk  1: reloaddisk in a new desktop    -1:set event and quit
    int reloadVDiskInNewDesktop(const std::string uuid, RELOAD_VDISK_TYPE iReloadDisk, LAUNCH_TYPE iLaunchType);
    int disconnectApp(const DISCONN_APP& stDisconnApp);
    int insertToMap(const std::string, const LAUNCH_THREAD_INFO&);
    int updateToMap(const std::string, const LAUNCH_THREAD_INFO&, bool bInsertIfNotFind = false);
    int eraseFromMap(const std::string);
    int findFromMap(const std::string, LAUNCH_THREAD_INFO*);
    int terminateAll(){return 0;}
    int appExit(const std::string&, LAUNCH_TYPE);//exit >0  not exit <=0
private:
   std::map<std::string, LAUNCH_THREAD_INFO> m_vctApp;
   CMutexOp m_mutex;   
};

#endif // CLAUNCHAPP_H
